#pragma once

#include<stack>
#include<deque>
#include "BehaviorEvent.h"
namespace BTEvent
{
	enum class EActionMode
	{
		Attack,
		Patrol,
		Runaway,
	};

	enum class EConditionMode
	{
		IsSeeEnemy,
		IsHealthLow,
		IsEnemyDead,
	};

	class BehaviorTree
	{
	public:
		BehaviorTree(Behavior* InRoot) :Root(InRoot) {}
		void Tick();
		bool HaveRoot() { return Root ? true : false; }
		void SetRoot(Behavior* InNode) { Root = InNode; }
		void Release() { Root->Release(); }
		bool Step();
		void Start(Behavior* Bh,BehaviorObserver* Observe);
		void Stop(Behavior* Bh,EStatus Result);
	private:
		//已激活行为列表
		std::deque<Behavior*> Behaviors;
		Behavior* Root;
	};


}


