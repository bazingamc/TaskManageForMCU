#include "task.h"
#include <iostream>

// 静态成员初始化
task* task::objects[task::MAX_OBJECTS] = { nullptr };
int task::objectCount = 0;
uint64_t task::now_time = 0;

task::task(const char* name, TaskFun fun)
{
	this->fun = fun;
	this->state = TASK_STATE_IDEL;
	this->name = name;

	if (objectCount < MAX_OBJECTS) 
	{
		objects[objectCount++] = this;
	}
	else 
	{
		// 超过容量可打印警告或处理
		//cout << "Warning: Max object limit reached!" << endl;
	}
}

task::~task()
{
	for (int i = 0; i < objectCount; i++) 
	{
		if (objects[i] == this) 
		{
			// 将最后一个对象替换到当前位置
			objects[i] = objects[objectCount - 1];
			objects[objectCount - 1] = nullptr;
			objectCount--;
			break;
		}
	}
}

void task::Start(TaskParam* param, uint8_t start_user_state)
{
	this->state = TASK_STATE_START;
	memcpy(&this->param, param, param->data_len);
	this->user_state = start_user_state;
}

void task::Stop()
{
	this->state = TASK_STATE_IDEL;
}

void task::SubtaskStart(task* subtask, TaskParam* param, uint8_t start_user_state, WhereToGO success_go, WhereToGO fail_go)
{
	this->subtask = subtask;
	this->state = TASK_STATE_WAITSUBTASK;
	this->subtask->Start(param, start_user_state);
	this->delay = 0;
	this->timeout = 0;
}

void task::SubtaskStart(task* subtask, TaskParam* param, uint8_t start_user_state, WhereToGO success_go, WhereToGO fail_go, uint32_t delay)
{
	this->subtask = subtask;
	this->state = TASK_STATE_WAITSUBTASK;
	this->subtask->Start(param, start_user_state);
	this->delay = delay;
	this->timeout = 0;
}

void task::SubtaskStart(task* subtask, TaskParam* param, uint8_t start_user_state, WhereToGO success_go, WhereToGO fail_go, uint32_t delay, uint32_t timeout)
{
	this->subtask = subtask;
	this->state = TASK_STATE_WAITSUBTASK;
	this->subtask->Start(param, start_user_state);
	this->delay = delay;
	this->timeout = timeout;
}

void task::Success()
{
	this->state = TASK_STATE_SUCCESS;
}

void task::Fail()
{
	this->state = TASK_STATE_FAIL;
}

void task::Delay(uint32_t ms, WhereToGO success_go)
{
	this->delay_time = ms;
	this->delay_start_time = task::now_time;
	this->state = TASK_STATE_DELAY;
	this->success_go = success_go;
}

void task::UserStateChange(uint8_t user_state)
{
	this->user_state = user_state;
}
void task::TransitionToNextState()
{
	this->user_state++;
}
uint8_t task::GetUserState()
{
	return this->user_state;	
}
bool task::IsTimeout(uint32_t ms, WhereToGO fail_go)
{
	if ((task::now_time - this->user_state_into_time) >= ms)
	{
		this->GoTo(fail_go);
		return true;
	}
	return false;
}

void task::GoTo(WhereToGO user_state)
{
	if (user_state < 256)
	{
		this->UserStateChange(user_state);
	}
	else
	{
		switch (user_state)
		{
		case WHERE_NEXT:
			this->TransitionToNextState();
			break;
		case WHERE_SUCCESS:
			this->Success();
			break;
		case WHERE_FAIL:
			this->Fail();
			break;
		default:
			// Do nothing
			break;
		}
	}
}

void task::TaskRun()
{
	for (uint16_t i = 0; i < task::objectCount; i++)
	{
		task* currentTask = task::objects[i];
		switch (currentTask->state)
		{
		case TASK_STATE_START:
			currentTask->state = TASK_STATE_RUN;
			currentTask->task_start_time = task::now_time;
			if (currentTask->father)
			{
				LOG("T:%s start, Father:%s\r\n", currentTask->name, currentTask->father->name);
			}
			else
			{
				LOG("T:%s start\r\n", currentTask->name);
			}
			break;
		case TASK_STATE_RUN:
			if (currentTask->last_user_state != currentTask->user_state)
			{
				LOG("T:%s user state: %d -> %d\r\n", currentTask->name, currentTask->last_user_state, currentTask->user_state);
				currentTask->last_user_state = currentTask->user_state;
				currentTask->user_state_into_time = task::now_time;
			}
			currentTask->fun(currentTask, &currentTask->param);
			break;
		case TASK_STATE_DELAY:
			if (task::now_time - currentTask->delay_start_time >= currentTask->delay_time)
			{
				currentTask->state = TASK_STATE_RUN;
				currentTask->GoTo(currentTask->success_go);
			}
			break;
		case TASK_STATE_WAITSUBTASK:
			if (currentTask->subtask->state == TASK_STATE_IDEL)
			{
				if (currentTask->delay)
				{
					currentTask->Delay(currentTask->delay, currentTask->success_go);
				}
				else
				{
					currentTask->state = TASK_STATE_RUN;
					currentTask->GoTo(currentTask->success_go);
				}
			}
			else if (currentTask->subtask->state == TASK_STATE_FAIL)
			{
				if (currentTask->delay)
				{
					currentTask->Delay(currentTask->delay, currentTask->fail_go);
				}
				else
				{
					currentTask->state = TASK_STATE_RUN;
					currentTask->GoTo(currentTask->fail_go);
				}
			}
			else if (currentTask->timeout != 0 && (task::now_time - currentTask->user_state_into_time) >= currentTask->timeout)
			{
				LOG("T:%s subtask %s timeout\r\n", currentTask->name, currentTask->subtask->name);
				currentTask->state = TASK_STATE_RUN;
				currentTask->GoTo(currentTask->fail_go);
			}
			break;
		case TASK_STATE_SUCCESS:
			LOG("T:%s finish, time %dms\r\n", currentTask->name, task::now_time - currentTask->task_start_time);
			currentTask->state = TASK_STATE_IDEL;
			break;
		case TASK_STATE_FAIL:
			LOG("T:%s fail, time %dms\r\n", currentTask->name, task::now_time - currentTask->task_start_time);
			currentTask->state = TASK_STATE_ERROR;
			break;

		default:
			break;
		}
	}
}