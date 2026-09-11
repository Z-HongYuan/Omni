// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Data/ExperienceSystemTags.h"

namespace ExperienceSystemTags
{
	// 初始化状态链条,由 ModularGameplay 使用
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InitState_Spawned, "InitState.Spawned", "初始化状态链: 组件已生成")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InitState_DataAvailable, "InitState.DataAvailable", "初始化状态链: 组件数据可用")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InitState_DataInitialized, "InitState.DataInitialized", "初始化状态链: 组件数据初始化")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InitState_GameplayReady, "InitState.GameplayReady", "初始化状态链: 组件游戏准备就绪")
	const TArray<FGameplayTag> ComponentStateChain = {
		ExperienceSystemTags::TAG_InitState_Spawned,
		ExperienceSystemTags::TAG_InitState_DataAvailable,
		ExperienceSystemTags::TAG_InitState_DataInitialized,
		ExperienceSystemTags::TAG_InitState_GameplayReady,
	};

	// 技能的特性/能力标签
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Behavior_AvoidDeathClear, "Ability.Behavior.AvoidDeathClear", "能力会在Pawn卸载时避免清除,意味着能力需要手动移除")
}
