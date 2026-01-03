#pragma once
#include <cstdint> 


#define LOG(str, ...)		printf(str, ##__VA_ARGS__);

class task;

typedef enum//任务状态
{
	TASK_STATE_IDEL,
	TASK_STATE_RUN,
	TASK_STATE_ERROR,
	TASK_STATE_DELAY,
	TASK_STATE_WAITEVENT,
	TASK_STATE_START,
	TASK_STATE_SUCCESS,
	TASK_STATE_FAIL,
	TASK_STATE_WAITSUBTASK,
} TaskState;

typedef struct
{
	task* father;
	void* data;
	int data_len;
}TaskParam;

typedef void (*TaskFun)(task* self, TaskParam* param) ;

typedef enum
{
	//0-255 跳转user_state

	WHERE_NEXT = 256,
	WHERE_NULL,
	WHERE_SUCCESS,
	WHERE_FAIL
}WhereToGO;

class task
{
public:

	task(const char* name, TaskFun fun);
	~task();

	void Start(TaskParam* param, uint8_t start_user_state);
	void Stop();
	void Success();
	void Fail();
	void SubtaskStart(task* subtask, TaskParam* param, uint8_t start_user_state, WhereToGO success_go, WhereToGO fail_go);
	void SubtaskStart(task* subtask, TaskParam* param, uint8_t start_user_state, WhereToGO success_go, WhereToGO fail_go, uint32_t delay);
	void SubtaskStart(task* subtask, TaskParam* param, uint8_t start_user_state, WhereToGO success_go, WhereToGO fail_go, uint32_t delay, uint32_t timeout);
	void Delay(uint32_t ms, WhereToGO success_go);
	void UserStateChange(uint8_t user_state);
	void TransitionToNextState(); 
	uint8_t GetUserState();
	bool IsTimeout(uint32_t ms, WhereToGO fail_go);

	//任务调度(在主循环中调用)
	static void TaskRun();

	//系统时间
	static uint64_t now_time;

private:
	//基本属性
	const char* name;
	TaskState state;
	TaskFun fun;
	task* father;
	TaskParam param;
	
	//用于遍历所有任务
	static const int MAX_OBJECTS = 64;   // 最大对象数量
	static task* objects[MAX_OBJECTS]; // 静态数组保存对象指针
	static int objectCount;               // 当前对象数量

	//状态机
	uint8_t user_state;
	uint8_t last_user_state;//上一次状态值
	
	//任务执行时间
	uint32_t task_start_time;
	uint64_t user_state_into_time;//进入当前状态的时间

	//延迟
	uint32_t delay_start_time;
	uint32_t delay_time;
	
	//去向
	uint32_t delay;
	uint32_t timeout;
	task* subtask;
	WhereToGO success_go;
	WhereToGO fail_go;
	
	//私有方法
	void GoTo(WhereToGO user_state);
};



