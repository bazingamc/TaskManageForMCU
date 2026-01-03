MCU裸机状态机任务管理框架

一个用于 裸机 MCU 的轻量级状态机任务管理框架，采用 C++ 实现，也可在 C 语言项目 中使用。框架设计遵循面向对象思想，同时兼顾嵌入式系统资源受限的特点，便于快速构建可靠的任务管理和多状态控制逻辑。

功能特点

轻量高效：专为裸机 MCU 设计，无需 RTOS 支持，占用资源小。

状态机驱动：每个任务可独立管理状态，支持状态切换、超时检测、事件触发。

多任务管理：框架可管理多个任务，任务间通信灵活。

易扩展：支持用户自定义任务参数、状态。

快速开始：

1.导入代码文件：
```
task.cpp  task.h
```

2.创建任务：
```
task t1("t1", task1);
```

3.编写任务实体：
将一个任务拆分为多个步骤，每个步骤使用一个case执行，任务中禁止阻塞延迟
```
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
```

4.在1ms定时器中挂载时间变量
```
task::now_time ++;
```

5.启动任务：
```
TaskParam p1;
p1.father = nullptr;
p1.data = nullptr;
p1.data_len = 0;
t1.Start(&p1, 0);//启动任务
```


