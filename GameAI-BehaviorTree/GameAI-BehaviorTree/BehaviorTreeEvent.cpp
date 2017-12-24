#include "stdafx.h"
#include "BehaviorTreeEvent.h"
#include<assert.h>

using namespace BTEvent;

void BehaviorTree::Tick()
{
	//将更新结束标记插入任务列表
	Behaviors.push_back(nullptr);
	while (Step())
	{
	}
}

bool BehaviorTree :: Step()
{
	Behavior* Current = Behaviors.front();
	Behaviors.pop_front();
	//如果遇到更新结束标记则停止
	if (Current == nullptr)
		return false;
	//执行行为更新
	Current->Tick();
	//如果该任务被终止则执行监察函数
	if (Current->IsTerminate() && Current->Observer)
	{
		Current->Observer(Current->GetStatus());
	}
	//否则将其插入队列等待下次tick处理
	else
	{
		Behaviors.push_back(Current);
	}
}

void BehaviorTree::Start(Behavior* Bh, BehaviorObserver* Observe)
{
	if (Observe)
	{
		Bh->Observer = *Observe;
	}
	Behaviors.push_front(Bh);
}
void BehaviorTree::Stop(Behavior* Bh, EStatus Result)
{
	assert(Result != EStatus::Running);
	Bh->SetStatus(Result);
	if (Bh->Observer)
	{
		Bh->Observer(Result);
	}
}





