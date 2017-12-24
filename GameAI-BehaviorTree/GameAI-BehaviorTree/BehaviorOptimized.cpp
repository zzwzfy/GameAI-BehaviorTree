#include "stdafx.h"
#include "BehaviorOptimized.h"
#include <algorithm>

extern std::_Binder<std::_Unforced, std::uniform_int_distribution<>&, std::default_random_engine&> dice;




using namespace BTOptimized;

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

EStatus Repeat::Update()
{
	while (true)
	{
		Child->Tick();
		if (Child->IsRunning())return EStatus::Success;
		if (Child->IsFailuer())return EStatus::Failure;
		if (++Count == Limited)return EStatus::Success;
		Child->Reset();
	}
	return EStatus::Invalid;
}

void Composite::RemoveChild(size_t InChild)
{
	Children[InChild] = 0;
}

EStatus Sequence::Update()
{
	while (true)
	{
		EStatus s = GetChild(CurrChild)->Tick();
		//如果执行成功了就继续执行，否则返回
		if (s != EStatus::Success)
			return s;
		if (++CurrChild == ChildrenCount)
			return EStatus::Success;
	}
	return EStatus::Invalid;  //循环意外终止
}

EStatus Selector::Update()
{
	while (true)
	{
		EStatus s = GetChild(CurrChild)->Tick();
		if (s != EStatus::Failure)
			return s;
		//如果执行失败了就继续执行，否则返回
		if (++CurrChild == ChildrenCount)
			return EStatus::Failure;
	}
	return EStatus::Invalid;  //循环意外终止
}

EStatus Parallel::Update()
{
	int SuccessCount = 0, FailureCount = 0;
	for (int i = 0; i < ChildrenCount;++i)
	{
		//如果行为已经终止则不再执行该行为
		if (!GetChild(i)->IsTerminate())
			GetChild(i)->Tick();

		if (GetChild(i)->IsSuccess())
		{
			++SuccessCount;
			if (SucessPolicy == EPolicy::RequireOne)
			{
				GetChild(i)->Reset();
				return EStatus::Success;
			}

		}

		if (GetChild(i)->IsFailuer())
		{
			++FailureCount;
			if (FailurePolicy == EPolicy::RequireOne)
			{
				GetChild(i)->Reset();
				return EStatus::Failure;
			}
		}
	}

	if (FailurePolicy == EPolicy::RequireAll&&FailureCount == ChildrenCount)
	{
		for (int i = 0; i < ChildrenCount;++i)
		{
			GetChild(i)->Reset();
		}

		return EStatus::Failure;
	}
	if (SucessPolicy == EPolicy::RequireAll&&SuccessCount == ChildrenCount)
	{
		for (int i = 0; i < ChildrenCount; ++i)
		{
			GetChild(i)->Reset();
		}
		return EStatus::Success;
	}

	return EStatus::Running;
}

void Parallel::OnTerminate(EStatus InStatus)
{
	for (int i = 0; i < ChildrenCount; ++i)
	{
		if (GetChild(i)->IsRunning())
			GetChild(i)->Abort();
	}
}

EStatus ActiveSelector::Update()
{
	//每次执行前先保存的当前节点
	uint16_t Previous = CurrChild;
	//调用父类OnInlitiallize函数让选择器每次重新选取节点
	Selector::OnInitialize();
	EStatus result = Selector::Update();
	//如果优先级更高的节点成功执行或者原节点执行失败则终止当前节点的执行
	if (Previous != ChildrenCount&&CurrChild != Previous)
	{
		GetChild(Previous)->Abort();
	}

	return result;
}

EStatus Condition_IsSeeEnemy::Update()
{
	if (dice() > 50)
	{
		std::cout << "See enemy!" << std::endl;
		return !IsNegation ? EStatus::Success : EStatus::Failure;
	}

	else
	{
		std::cout << "Not see enemy" << std::endl;
		return !IsNegation ? EStatus::Failure : EStatus::Success;
	}

}

EStatus Condition_IsHealthLow::Update()
{
	if (dice() > 80)
	{
		std::cout << "Health is low" << std::endl;
		return !IsNegation ? EStatus::Success : EStatus::Failure;
	}

	else
	{
		std::cout << "Health is not low" << std::endl;
		return !IsNegation ? EStatus::Failure : EStatus::Success;
	}
}

EStatus Condition_IsEnemyDead::Update()
{
	if (dice() > 50)
	{
		std::cout << "Enemy is Dead" << std::endl;
		return !IsNegation ? EStatus::Success : EStatus::Failure;
	}

	else
	{
		std::cout << "Enemy is not Dead" << std::endl;
		return !IsNegation ? EStatus::Failure : EStatus::Success;
	}
}

EStatus Action_Attack::Update()
{
	std::cout << "Action_Attack " << std::endl;
	return EStatus::Success;
}

EStatus Action_Patrol::Update()
{
	std::cout << "Action_Patrol" << std::endl;
	return EStatus::Success;
}

EStatus Action_Runaway::Update()
{
	std::cout << "Action_Runaway" << std::endl;
	return EStatus::Success;
}