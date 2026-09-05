// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Data/CustomAbilitySystemTags.h"


#define UE_API CUSTOMABILITYSYSTEM_API

namespace CustomAbilitySystemTags
{
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gameplay_AbilityInputBlocked, "Gameplay.AbilityInputBlocked", "技能系统: 阻止所有的主动技能输入/触发");

	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_SimpleTextFailure_Message, "Ability.SimpleTextFailure.Message", "技能系统: 当失败时,传递简单的文本消息,用于Messaging");
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_PlayMontageFailure_Message, "Ability.PlayMontageOnActivateFailure.Message", "技能系统: 当失败时,传递播放蒙太奇消息,用于Messaging");

	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_ActivateFail_ActivationGroup, "Ability.ActivateFail.ActivationGroup", "技能系统: 技能因为技能激活组的原因,激活失败");
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_ActivateFail_IsDead, "Ability.ActivateFail.IsDead", "技能系统: 技能因为玩家死亡的原因,激活失败");

	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Status_Death, "Status.Death", "技能系统: 死亡状态");

	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Behavior_SurvivesDeath, "Ability.Behavior.SurvivesDeath", "技能系统: 技能持续存在,豁免死亡时清除");
}

#undef UE_API
