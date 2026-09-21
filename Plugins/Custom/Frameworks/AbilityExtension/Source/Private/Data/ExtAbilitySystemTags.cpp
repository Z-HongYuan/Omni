// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Data/ExtAbilitySystemTags.h"


#define UE_API ABILITYEXTENSION_API

namespace ExtAbilitySystemTags
{
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_DataTag_SetByCaller_Damage, "DataTag.SetByCaller.Damage", "通过 SetByCaller 传递本次伤害数值，作为 GE 与调用者约定的数据标识");
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_DataTag_SetByCaller_Heal, "DataTag.SetByCaller.Heal", "通过 SetByCaller 传递本次治疗数值，作为 GE 与调用者约定的数据标识");

	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gameplay_AbilityInputBlocked, "Gameplay.AbilityInputBlocked", "技能系统: 阻止所有的主动技能输入/触发");
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEvent_Death, "GameplayEvent.Death", "技能系统: 生命归零时触发死亡技能");
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gameplay_DamageSelfDestruct, "Gameplay.DamageSelfDestruct", "伤害 GE 来源: 自毁");
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Gameplay_FellOutOfWorld, "Gameplay.FellOutOfWorld", "伤害 GE 来源: 掉出世界");

	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_SimpleTextFailure_Message, "Ability.SimpleTextFailure.Message", "技能系统: 当失败时,传递简单的文本消息,用于Messaging");
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_PlayMontageFailure_Message, "Ability.PlayMontageOnActivateFailure.Message", "技能系统: 当失败时,传递播放蒙太奇消息,用于Messaging");

	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_ActivateFail_ActivationGroup, "Ability.ActivateFail.ActivationGroup", "技能系统: 技能因为技能激活组的原因,激活失败");
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_ActivateFail_IsDead, "Ability.ActivateFail.IsDead", "技能系统: 技能因为玩家死亡的原因,激活失败");

	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Status_Death, "Status.Death", "技能系统: 死亡状态");
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Status_Death_Dying, "Status.Death.Dying", "技能系统: 死亡流程已开始");
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Status_Death_Dead, "Status.Death.Dead", "技能系统: 死亡流程已结束");

	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Behavior_AvoidDeathClear, "Ability.Behavior.AvoidDeathClear", "技能系统: 豁免死亡开始及 Pawn 注销 ASC 时的批量取消，不影响其他取消规则");
}

#undef UE_API
