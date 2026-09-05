// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

#define UE_API EXPERIENCESYSTEM_API

namespace ExperienceSystemTags
{
	// 初始化状态链条,由 ModularGameplay 使用
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_InitState_Spawned)
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_InitState_DataAvailable)
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_InitState_DataInitialized)
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_InitState_GameplayReady)
	UE_API extern const TArray<FGameplayTag> ComponentStateChain;

	// 技能的特性/能力标签
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Behavior_AvoidDeathClear)
}

#undef UE_API
