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

	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Behavior_SurvivesDeath)
}

#undef UE_API
