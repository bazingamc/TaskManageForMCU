// TaskManage.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <windows.h>
#include <mmsystem.h>
#include "task.h"

#pragma comment(lib, "winmm.lib") // 链接多媒体库



// 回调函数
void CALLBACK TimerProc(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2) {
	task::now_time +=1000;
}

void timer_init()
{
	// 设置定时器分辨率为 1ms
	timeBeginPeriod(1);

	// 创建周期性定时器，间隔 1ms
	MMRESULT timerId = timeSetEvent(
		1000,           // 定时间隔 1ms
		0,           // 分辨率，0表示最大精度
		TimerProc,   // 回调函数
		0,           // 用户数据
		TIME_PERIODIC // 周期性定时器
	);

	if (timerId == 0) {
		std::cout << "Failed to create timer\n";
		return;
	}
}


void task1(task* self, TaskParam* param);
void task2(task* self, TaskParam* param);
task t1("t1", task1);
task t2("t2", task2);

void task1(task* self, TaskParam* param)
{
	switch (self->GetUserState())
	{
	case 0:
		self->TransitionToNextState();
		break;
	case 1:
		self->Delay(2000, WHERE_NEXT);
		break;
	case 2:
		self->SubtaskStart(&t2, param, 0, WHERE_NEXT, WHERE_FAIL);
		break;
	case 3:
		self->UserStateChange(0);

	default:
		break;
	}
}

void task2(task* self, TaskParam* param)
{
	switch (self->GetUserState())
	{
	case 0:
		self->UserStateChange(1);
		break;
	case 1:
		self->Delay(2000, WHERE_NEXT);
		break;
	case 2:
		std::cout << "task2 success!\n";
		self->Success();
		break;
	default:
		break;
	}
}


int main()
{
	timer_init();// 初始化定时器

	TaskParam p1;
	p1.father = nullptr;
	p1.data = nullptr;
	p1.data_len = 0;
	t1.Start(&p1, 0);//启动任务

	while (1)
	{
		task::TaskRun();
	}
}
