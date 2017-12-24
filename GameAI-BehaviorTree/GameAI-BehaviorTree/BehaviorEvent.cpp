#include "stdafx.h"
#include "BehaviorEvent.h"
#include "BehaviorTreeEvent.h"
#include <algorithm>
#include <random>
#include <functional>
#include<assert.h>

extern std::_Binder<std::_Unforced, std::uniform_int_distribution<>&, std::default_random_engine&> dice;


using namespace BTEvent;

EStatus Behavior::Tick()
{
	//update方法被首次调用前执行OnInitlize方法，每次行为树更新时调用一次update方法
	//当刚刚更新的行为不再运行时调用OnTerminate方法
	if (Status != EStatus::Running)
	{
		OnInitialize();
	}

	Status = Update();

	if (Status != EStatus::Running)
	{
		OnTerminate(Status);
	}

	return Status;
}


void Composite::RemoveChild(Behavior* InChild)
{
	auto it = std::find(Children.begin(), Children.end(), InChild);
	if (it != Children.end())
	{
		Children.erase(it);
	}
}

void Sequence::OnInitialize()
{
	CurrChild = Children.begin();
	BehaviorObserver observer = std::bind(&Sequence::OnChildComplete, this, std::placeholders::_1);
	Tree->Start(*CurrChild, &observer);
}


void Sequence::OnChildComplete(EStatus Status)
{
	Behavior* child = *CurrChild;
	if (child->IsFailuer())
	{
		m_pBehaviorTree->Stop(this, EStatus::Failure);
		return;
	}

	assert(child->GetStatus() == EStatus::Success);
	if (++CurrChild == Children.end())
	{
		Tree->Stop(this, EStatus::Success);
	}
	else
	{
		BehaviorObserver observer = std::bind(&Sequence::OnChildComplete, this, std::placeholders::_1);
		Tree->Start(*CurrChild, &observer);
	}
}