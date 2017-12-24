#pragma once
#include<vector>
#include<string>
#include<iostream>


namespace BT
{
	//行为树返回状态
	enum class EStatus :uint8_t
	{
		Invalid,   //初始状态
		Success,   //成功
		Failure,   //失败
		Running,   //运行
		Aborted,   //终止
	};

	//Parallel节点成功与失败的要求，是全部成功/失败，还是一个成功/失败
	enum class EPolicy :uint8_t
	{
		RequireOne,
		RequireAll,
	};

	class Behavior
	{
	public:
		//释放对象所占资源
		virtual void Release() = 0;
		//包装函数，防止打破调用契约
		EStatus Tick();

		EStatus GetStatus() { return Status; }

		void Reset() { Status = EStatus::Invalid; }
		void Abort() { OnTerminate(EStatus::Aborted); Status = EStatus::Aborted; }

		bool IsTerminate() { return Status == EStatus::Success || Status == EStatus::Failure; }
		bool IsRunning() { return Status == EStatus::Running; }
		bool IsSuccess() { return Status == EStatus::Success; }
		bool IsFailuer() { return Status == EStatus::Failure; }

		virtual std::string Name() = 0;
		virtual void AddChild(Behavior* Child) {};

	protected:
		//创建对象请调用Create()释放对象请调用Release()
		Behavior() :Status(EStatus::Invalid) {}
		virtual ~Behavior() {}

		virtual void OnInitialize() {};
		virtual EStatus Update() = 0;
		virtual void OnTerminate(EStatus Status) {};

	protected:
		EStatus Status;
	};

	//装饰器
	class Decorator :public Behavior
	{
	public:
		virtual void AddChild(Behavior* InChild) { Child = InChild; }
	protected:
		Decorator() {}
		virtual ~Decorator() {}
		Behavior* Child;
	};

	//重复执行子节点的装饰器
	class Repeat :public Decorator
	{
	public:

		static Behavior* Create(int InLimited) { return new Repeat(InLimited); }
		virtual void Release() { Child->Release(); delete this; }
		virtual std::string Name() override { return "Repeat"; }

	protected:
		Repeat(int InLimited) :Limited(InLimited) {}
		virtual ~Repeat() {}
		virtual void OnInitialize() { Count = 0; }
		virtual EStatus Update()override;
		virtual Behavior* Create() { return nullptr; }
	protected:
		int Limited = 3;
		int Count = 0;
	};

	//复合节点基类
	class Composite :public Behavior
	{
	public:
		virtual void AddChild(Behavior* InChild) override { Children.push_back(InChild); }
		void RemoveChild(Behavior* InChild);
		void ClearChild() { Children.clear(); }
		virtual void Release()
		{
			for (auto it : Children)
			{
				it->Release();
			}

			delete this;
		}

	protected:
		Composite() {}
		virtual ~Composite() {}
		using Behaviors = std::vector<Behavior*>;
		Behaviors Children;
	};

	//顺序器：依次执行所有节点直到其中一个失败或者全部成功位置
	class Sequence :public Composite
	{
	public:
		virtual std::string Name() override { return "Sequence"; }
		static Behavior* Create() { return new Sequence(); }
	protected:
		Sequence() {}
		virtual ~Sequence() {}
		virtual void OnInitialize() override { CurrChild = Children.begin(); }
		virtual EStatus Update() override;

	protected:
		Behaviors::iterator CurrChild;
	};

	//过滤器：在特定条件下拒绝执行子行为的节点，可以通过将条件放在头部的特殊顺序器来实现
	class Filter :public Sequence
	{
	public:
		static Behavior* Create() { return new Filter(); }
		void AddCondition(Behavior* Condition) { Children.insert(Children.begin(), Condition); }
		void AddAction(Behavior* Action) { Children.push_back(Action); }
		virtual std::string Name() override { return "Fliter"; }

	protected:
		Filter() {}
		virtual ~Filter() {}
	};

	//选择器:依次执行每个子节点直到其中一个执行成功或者返回Running状态
	class Selector :public Composite
	{
	public:
		static Behavior* Create() { return new Selector(); }
		virtual std::string Name() override { return "Selector"; }

	protected:
		Selector() {}
		virtual ~Selector() {}
		virtual void OnInitialize() override { CurrChild = Children.begin(); }
		virtual EStatus Update() override;

	protected:
		Behaviors::iterator CurrChild;
	};

	//并行器：多个行为并行执行
	class Parallel :public Composite
	{
	public:
		static Behavior* Create(EPolicy InSucess, EPolicy InFailure) { return new Parallel(InSucess, InFailure); }
		virtual std::string Name() override { return "Parallel"; }

	protected:
		Parallel(EPolicy InSucess, EPolicy InFailure) :SucessPolicy(InSucess), FailurePolicy(InFailure) {}
		virtual ~Parallel() {}
		virtual EStatus Update() override;
		virtual void OnTerminate(EStatus InStatus) override;

	protected:
		EPolicy SucessPolicy;
		EPolicy FailurePolicy;
	};

	//监视器:持续检查假设条件的有效性，一旦条件不满足，即时退出当前行为
	class Monitor :public Parallel
	{
	public:
		static Behavior* Create(EPolicy InSucess, EPolicy InFailure) { return new Monitor(InSucess, InFailure); }
		void AddCondition(Behavior* Condition) { Children.insert(Children.begin(), Condition); }
		void AddAction(Behavior* Action) { Children.push_back(Action); }
		virtual std::string Name() override { return "Monitor"; }

	protected:
		Monitor(EPolicy InSucess, EPolicy InFailure) :Parallel(InSucess, InFailure) {}
		virtual ~Monitor() {}
	};

	//主动选择器：执行过程中不断检查高优先级行为的可行性,高优先级行为可中断低优先级行为
	class ActiveSelector :public Selector
	{
	public:
		static Behavior* Create() { return new ActiveSelector(); }
		//开始时把当前节点置为end
		virtual void OnInitialize() override { CurrChild = Children.end(); }
		virtual std::string Name() override { return "ActiveSelector"; }
	protected:
		ActiveSelector() {}
		virtual ~ActiveSelector() {}
		virtual EStatus Update() override;
	};

	//条件基类
	class Condition :public Behavior
	{
	public:
		virtual void Release() { delete this; }

	protected:
		Condition(bool InIsNegation) :IsNegation(InIsNegation) {}
		virtual ~Condition() {}

	protected:
		//是否取反
		bool  IsNegation = false;
	};

	//动作基类
	class Action :public Behavior
	{
	public:
		virtual void Release() { delete this; }

	protected:
		Action() {}
		virtual ~Action() {}
	};

	//看见敌人条件
	class Condition_IsSeeEnemy :public Condition
	{
	public:
		static Behavior* Create(bool InIsNegation) { return new Condition_IsSeeEnemy(InIsNegation); }
		virtual std::string Name() override { return "Condtion_IsSeeEnemy"; }

	protected:
		Condition_IsSeeEnemy(bool InIsNegation) :Condition(InIsNegation) {}
		virtual ~Condition_IsSeeEnemy() {}
		virtual EStatus Update() override;
	};

	//血量低条件
	class Condition_IsHealthLow :public Condition
	{
	public:
		static Behavior* Create(bool InIsNegation) { return new Condition_IsHealthLow(InIsNegation); }
		virtual std::string Name() override { return "Condition_IsHealthLow"; }

	protected:
		Condition_IsHealthLow(bool InIsNegation) :Condition(InIsNegation) {}
		virtual ~Condition_IsHealthLow() {}
		virtual EStatus Update() override;

	};

	// 敌人死亡条件
	class Condition_IsEnemyDead :public Condition
	{
	public:
		static Behavior* Create(bool InIsNegation) { return new Condition_IsEnemyDead(InIsNegation); }
		virtual std::string Name() override { return "Condition_IsHealthLow"; }

	protected:
		Condition_IsEnemyDead(bool InIsNegation) :Condition(InIsNegation) {}
		virtual ~Condition_IsEnemyDead() {}
		virtual EStatus Update() override;

	};

	//攻击动作
	class Action_Attack :public Action
	{
	public:
		static Behavior* Create() { return new Action_Attack(); }
		virtual std::string Name() override { return "Action_Attack"; }

	protected:
		Action_Attack() {}
		virtual ~Action_Attack() {}
		virtual EStatus Update() override;
	};

	//逃跑动作
	class Action_Runaway :public Action
	{
	public:
		static Behavior* Create() { return new Action_Runaway(); }
		virtual std::string Name() override { return "Action_Runaway"; }

	protected:
		Action_Runaway() {}
		virtual ~Action_Runaway() {}
		virtual EStatus Update() override;
	};

	//巡逻动作
	class Action_Patrol :public Action
	{
	public:
		static Behavior* Create() { return new Action_Patrol(); }
		virtual std::string Name() override { return "Action_Patrol"; }

	protected:
		Action_Patrol() {}
		virtual ~Action_Patrol() {}
		virtual EStatus Update() override;
	};
}



