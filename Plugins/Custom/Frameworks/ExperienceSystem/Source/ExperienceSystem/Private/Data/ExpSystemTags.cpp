// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Data/ExpSystemTags.h"

namespace ExpSystemTags
{
	// 初始化状态链条,由 ModularGameplay 使用
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InitState_Spawned, "InitState.Spawned", "初始化状态链: 组件已生成")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InitState_DataAvailable, "InitState.DataAvailable", "初始化状态链: 组件数据可用")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InitState_DataInitialized, "InitState.DataInitialized", "初始化状态链: 组件数据初始化")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InitState_GameplayReady, "InitState.GameplayReady", "初始化状态链: 组件游戏准备就绪")
	const TArray<FGameplayTag> ComponentStateChain = {
		ExpSystemTags::TAG_InitState_Spawned,
		ExpSystemTags::TAG_InitState_DataAvailable,
		ExpSystemTags::TAG_InitState_DataInitialized,
		ExpSystemTags::TAG_InitState_GameplayReady,
	};
}
