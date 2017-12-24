#include "stdafx.h"
#include "BehaviorTree.h"
#include<assert.h>

using namespace BT;

void BehaviorTree::Tick()
{	
	Root->Tick();
}

BehaviorTreeBuilder* BehaviorTreeBuilder::Sequence()
{
	Behavior* Sq=Sequence::Create();
	AddBehavior(Sq);
	return this;
}
BehaviorTreeBuilder* BehaviorTreeBuilder::Action(EActionMode ActionModes)
{
	//根据输入参数创建不同动作节点
	Behavior* Ac;
	switch (ActionModes)
	{
	case EActionMode::Attack:
		Ac=Action_Attack::Create();
		break;

	case  EActionMode::Patrol:
		Ac= Action_Patrol::Create();
		break;

	case EActionMode::Runaway:
		Ac=Action_Runaway::Create();
		break;

	default:
		Ac = nullptr;
		break;
	}
	
	AddBehavior(Ac);

	return this;
}

BehaviorTreeBuilder* BehaviorTreeBuilder::Condition(EConditionMode ConditionMode,bool IsNegation)
{
	//根据输入参数创建不同条件节点
	Behavior* Cd;
	switch (ConditionMode)
	{
	case EConditionMode::IsSeeEnemy:
		Cd=Condition_IsSeeEnemy::Create(IsNegation);
		break;

	case  EConditionMode::IsHealthLow:
		Cd=Condition_IsHealthLow::Create(IsNegation);
		break;

	case EConditionMode::IsEnemyDead:
		Cd = Condition_IsEnemyDead::Create(IsNegation);
		break;

	default:
		Cd = nullptr;
		break;
	}

	AddBehavior(Cd);

	return this;
}

BehaviorTreeBuilder* BehaviorTreeBuilder::Selector()
{
	Behavior* St=Selector::Create();

	AddBehavior(St);

	return this;
}

BehaviorTreeBuilder* BehaviorTreeBuilder::Repeat(int RepeatNum)
{
	Behavior* Rp=Repeat::Create(RepeatNum);

	AddBehavior(Rp);

	return this;
}

BehaviorTreeBuilder* BehaviorTreeBuilder::ActiveSelector()
{
	Behavior* Ast = ActiveSelector::Create();

	AddBehavior(Ast);

	return this;
}

BehaviorTreeBuilder* BehaviorTreeBuilder::Filter()
{
	Behavior* Ft = Filter::Create();

	AddBehavior(Ft);

	return this;
}

BehaviorTreeBuilder* BehaviorTreeBuilder::Parallel(EPolicy InSucess, EPolicy InFailure)
{
	Behavior* Pl=Parallel::Create(InSucess,InFailure);

	AddBehavior(Pl);

	return this;
}
BehaviorTreeBuilder* BehaviorTreeBuilder::Monitor(EPolicy InSucess, EPolicy InFailure)
{
	Behavior* Mt=Monitor::Create(InSucess,InFailure);

	AddBehavior(Mt);

	return this;
}

BehaviorTreeBuilder* BehaviorTreeBuilder::Back()
{
	NodeStack.pop();
	return this;
}

BehaviorTree* BehaviorTreeBuilder::End()
{
	while (!NodeStack.empty())
	{
		NodeStack.pop();
	}
	BehaviorTree* Tmp= new BehaviorTree(TreeRoot);
	TreeRoot = nullptr;
	return Tmp;
}

void BehaviorTreeBuilder::AddBehavior(Behavior* NewBehavior)
{
	assert(NewBehavior);
	//如果没有根节点设置新节点为根节点
	if (!TreeRoot)
	{
		TreeRoot=NewBehavior;
	}
	//否则设置新节点为堆栈顶部节点的子节点
	else
	{
		NodeStack.top()->AddChild(NewBehavior);
	}

	//将新节点压入堆栈
	NodeStack.push(NewBehavior);
}