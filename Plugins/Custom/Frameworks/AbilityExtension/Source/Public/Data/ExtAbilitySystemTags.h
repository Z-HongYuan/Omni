// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

#define UE_API ABILITYEXTENSION_API

namespace ExtAbilitySystemTags
{
	/** GE 的 SetByCaller 伤害数值标签。 */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_DataTag_SetByCaller_Damage);
	/** GE 的 SetByCaller 治疗数值标签。 */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_DataTag_SetByCaller_Heal);

	/** 存在于 ASC 时阻止能力输入，并清空待处理输入。 */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_AbilityInputBlocked);
	
	/** 触发死亡能力的 GameplayEvent 标签。 */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameplayEvent_Death);

	/** 目标 ASC 上的伤害免疫状态；自毁伤害不受其限制。 */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_DamageImmunity);
	
	/** 附加到伤害 GE 的资产标签，标识自毁原因。 */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_DamageSelfDestruct);
	/** 标识掉出世界引发的伤害原因。 */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_FellOutOfWorld);

	/** 能力激活失败时广播文本反馈的消息通道。 */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_SimpleTextFailure_Message)
	/** 能力激活失败时广播蒙太奇反馈的消息通道。 */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_PlayMontageFailure_Message)

	/** 激活组互斥导致能力激活失败的原因标签。 */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_ActivateFail_ActivationGroup)
	/** 死亡状态下被标签条件阻止激活的失败原因。 */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_ActivateFail_IsDead)

	/** 死亡状态的父标签。 */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Death)
	/** 死亡开始后设置的状态标签。 */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Death_Dying)
	/** 死亡完成后设置的状态标签。 */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Death_Dead)

	/** 豁免死亡开始和 Pawn 解绑 ASC 时的批量取消，不影响其他取消规则。 */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Behavior_AvoidDeathClear)
}

#undef UE_API
