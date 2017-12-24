#include "stdafx.h"
#include "Behavior.h"
#include <algorithm>
#include <random>
#include <functional>

std::random_device rd;
std::default_random_engine engine(rd());
std::uniform_int_distribution<> dis(1, 100);
//auto dice= std::bind(dis, engine);
std::_Binder<std::_Unforced, std::uniform_int_distribution<>&, std::default_random_engine&> dice = std::bind(dis, engine);


using namespace BT;

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

void Composite::RemoveChild(Behavior* InChild)
{
	auto it = std::find(Children.begin(), Children.end(), InChild);
	if (it != Children.end())
	{
		Children.erase(it);
	}
}

EStatus Sequence::Update()
{
	while (true)
	{
		EStatus s = (*CurrChild)->Tick();
		//如果执行成功了就继续执行，否则返回
		if (s != EStatus::Success)
			return s;
		if (++CurrChild == Children.end())
			return EStatus::Success;
	}
	return EStatus::Invalid;  //循环意外终止
}

EStatus Selector::Update()
{
	while (true)
	{
        EStatus s = (*CurrChild)->Tick();
		if (s != EStatus::Failure)
			return s;	
		//如果执行失败了就继续执行，否则返回
		if (++CurrChild == Children.end())
			return EStatus::Failure;
	}
	return EStatus::Invalid;  //循环意外终止
}

EStatus Parallel::Update()
{
	int SuccessCount = 0, FailureCount = 0;
	int ChildrenSize = Children.size();
	for (auto it : Children)
	{
		//如果行为已经终止则不再执行该行为
		if (!it->IsTerminate())
			it->Tick();

		if (it->IsSuccess())
		{
			++SuccessCount;
			if (SucessPolicy == EPolicy::RequireOne)
			{
				it->Reset();
				return EStatus::Success;
			}
				
		}

		if (it->IsFailuer())
		{
			++FailureCount;
			if (FailurePolicy == EPolicy::RequireOne)
			{
				it->Reset();
				return EStatus::Failure;
			}		
		}
	}

	if (FailurePolicy == EPolicy::RequireAll&&FailureCount == ChildrenSize)
	{
		for (auto it : Children)
		{
			it->Reset();
		}
		
		return EStatus::Failure;
	}
	if (SucessPolicy == EPolicy::RequireAll&&SuccessCount == ChildrenSize)
	{
		for (auto it : Children)
		{
			it->Reset();
		}
		return EStatus::Success;
	}

	return EStatus::Running;
}
	
void Parallel::OnTerminate(EStatus InStatus)
{
	 for (auto it : Children)
	{
		if (it->IsRunning())
			it->Abort();
	}
}

	EStatus ActiveSelector::Update()
	{
		//每次执行前先保存的当前节点
		Behaviors::iterator Previous = CurrChild;
		//调用父类OnInlitiallize函数让选择器每次重新选取节点
		Selector::OnInitialize();
		EStatus result = Selector::Update();
		//如果优先级更高的节点成功执行或者原节点执行失败则终止当前节点的执行
		if (Previous != Children.end()&CurrChild != Previous)
		{
			(*Previous)->Abort();	
		}

		return result;
	}

EStatus Condition_IsSeeEnemy::Update()
{
	if (dice() > 50)
	{
		std::cout << "See enemy!"<<std::endl;
		return !IsNegation? EStatus::Success:EStatus::Failure;
	}

	else
	{
		std::cout << "Not see enemy" << std::endl;
		return !IsNegation? EStatus::Failure:EStatus::Success;
	}

}

EStatus Condition_IsHealthLow::Update()
{
	if (dice() > 80)
	{
		std::cout << "Health is low" << std::endl;
		return !IsNegation? EStatus::Success:EStatus::Failure;
	}

	else
	{
		std::cout << "Health is not low" << std::endl;
		return !IsNegation? EStatus::Failure:EStatus::Success;
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