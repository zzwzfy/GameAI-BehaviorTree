// GameAI-BehaviorTree.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "BehaviorTree.h"
#include <random>
#include <functional>

int main()
{
	//构建行为树：角色一开始处于巡逻状态，一旦发现敌人，先检查自己生命值是否过低，如果是就逃跑，否则就攻击敌人，攻击过程中如果生命值过低也会中断攻击，立即逃跑，如果敌人死亡则立即停止攻击
	BT::BehaviorTreeBuilder* Builder = new BT::BehaviorTreeBuilder();
	BT::BehaviorTree* Bt=Builder
		->ActiveSelector()
            ->Sequence()
	    	    ->Condition(BT::EConditionMode::IsSeeEnemy,false)
		             ->Back()       
		        ->ActiveSelector()
		             ->	Sequence()
		                  ->Condition(BT::EConditionMode::IsHealthLow,false)
           	                   ->Back()
		                  ->Action(BT::EActionMode::Runaway)
		                        ->Back()
		                  ->Back()
		            ->Parallel(BT::EPolicy::RequireAll, BT::EPolicy::RequireOne)
		                  ->Condition(BT::EConditionMode::IsEnemyDead,true)
		                        ->Back()
		                  ->Action(BT::EActionMode::Attack)
		                        ->Back()
		                  ->Back()
		           ->Back()
		        ->Back()
		->Action(BT::EActionMode::Patrol)
    ->End();

	delete Builder;

	//模拟执行行为树
	for (int i = 0; i < 10; ++i)
	{
		Bt->Tick();
		std::cout << std::endl;
	}

	Bt->Release();
	
	system("pause");
    return 0;
}

