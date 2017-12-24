#pragma once
#include<vector>
#include<string>
#include<iostream>
#include<functional>



namespace BTEvent
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

	using BehaviorObserver = std::function<void(EStatus)>;


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
		void SetStatus(EStatus InStatus) { Status = InStatus; }
		void Reset() { Status = EStatus::Invalid; }
		void Abort() { OnTerminate(EStatus::Aborted); Status = EStatus::Aborted; }

		bool IsTerminate() { return Status == EStatus::Success || Status == EStatus::Failure; }
		bool IsRunning() { return Status == EStatus::Running; }
		bool IsSuccess() { return Status == EStatus::Success; }
		bool IsFailuer() { return Status == EStatus::Failure; }

		virtual std::string Name() = 0;
		virtual void AddChild(Behavior* Child) {};
	public:
		BehaviorObserver Observer;
	protected:
		//创建对象请调用Create()释放对象请调用Release()
		Behavior() :Status(EStatus::Invalid) {}
		virtual ~Behavior() {}

		virtual void OnInitialize() {};
		virtual EStatus Update() = 0;
		virtual void OnTerminate(EStatus Status) {};

	protected:
		EStatus Status;
		class BehaviorTree* Tree;

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
		void OnChildComplete(EStatus Status);
	protected:
		Sequence() {}
		virtual ~Sequence() {}
		virtual void OnInitialize() override;
		virtual EStatus Update() override;

	protected:
		Behaviors::iterator CurrChild;
		BehaviorTree* m_pBehaviorTree;
	};

}


