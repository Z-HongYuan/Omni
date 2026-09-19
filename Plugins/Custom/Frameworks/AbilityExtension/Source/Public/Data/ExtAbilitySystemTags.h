// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

#define UE_API ABILITYEXTENSION_API

namespace ExtAbilitySystemTags
{
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_AbilityInputBlocked);

	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_SimpleTextFailure_Message)
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_PlayMontageFailure_Message)

	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_ActivateFail_ActivationGroup)
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_ActivateFail_IsDead)

	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Death)

	// 豁免死亡开始和 Pawn 注销 ASC 时的批量取消，不影响技能授予或其他取消规则。
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Behavior_AvoidDeathClear)
}

#undef UE_API
