#pragma once

#include<stack>
#include "BehaviorOptimized.h"
namespace BTOptimized
{
	const size_t MaxBehaviorTreeMemory = 8192;

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
		BehaviorTree(Behavior* InRoot):Root(InRoot),Buffer(new uint8_t[MaxBehaviorTreeMemory]),Offset(0){}
		BehaviorTree():Buffer(new uint8_t[MaxBehaviorTreeMemory]), Offset(0) {}
		~BehaviorTree(){ delete[] Buffer; }
		void Tick();
		bool HaveRoot() { return Root ? true : false; }
		void SetRoot(Behavior* InNode) { Root = InNode; }

		template<typename T>
		T* Allocate()
		{
			T* Node = new((void*)((uintptr_t)Buffer + Offset)) T;
			Offset += sizeof(T);
			assert(Offset < MaxBehaviorTreeMemory);
			return Node;
		}

		template<typename T>
		T* Allocate(bool IsNegation	)
		{
			T* Node = new((void*)((uintptr_t)Buffer + Offset)) T(IsNegation);
			Offset += sizeof(T);
			assert(Offset < MaxBehaviorTreeMemory);
			return Node;
		}

		template<typename T>
		T* Allocate(int RepeatNum)
		{
			T* Node = new((void*)((uintptr_t)Buffer + Offset)) T(RepeatNum);
			Offset += sizeof(T);
			assert(Offset < MaxBehaviorTreeMemory);
			return Node;
		}

		template<typename T>
		T* Allocate(EPolicy InSucess, EPolicy InFailure)
		{
			T* Node = new((void*)((uintptr_t)Buffer + Offset)) T(InSucess,InFailure);
			Offset += sizeof(T);
			assert(Offset < MaxBehaviorTreeMemory);
			return Node;
		}

protected:
		Behavior* Root=nullptr;
		uint8_t* Buffer;
		size_t Offset;
	};

	//行为树构建器，用来构建一棵行为树,通过前序遍历方式配合Back()和End()方法进行构建
	class BehaviorTreeBuilder
	{
	public:
		BehaviorTreeBuilder() { Bt = new BehaviorTree(); }
		~BehaviorTreeBuilder() { delete Bt; }
		BehaviorTreeBuilder* Sequence();
		BehaviorTreeBuilder* Action(EActionMode ActionModes);
		BehaviorTreeBuilder* Condition(EConditionMode ConditionMode, bool IsNegation);
		BehaviorTreeBuilder* Selector();
		BehaviorTreeBuilder* Repeat(int RepeatNum);
		BehaviorTreeBuilder* ActiveSelector();
		BehaviorTreeBuilder* Parallel(EPolicy InSucess, EPolicy InFailure);
		BehaviorTreeBuilder* Back();
		BehaviorTree* End();

	private:
		void AddBehavior(Behavior* NewBehavior);

	private:
		BehaviorTree* Bt = nullptr;
		//用于存储节点的堆栈
		std::stack<Behavior*> NodeStack;
	};
}


