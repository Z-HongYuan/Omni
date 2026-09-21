// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

#define UE_API ABILITYEXTENSION_API

namespace ExtAbilitySystemTags
{
	/*
	 * 在GE中使用的标识性数据Tag
	 */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_DataTag_SetByCaller_Damage);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_DataTag_SetByCaller_Heal);

	/*
	 *  在ASC中,在输入处理管线中阻挡输入的触发
	 */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_AbilityInputBlocked);
	
	/*
	 * 用于触发技能的 GameplayEvent事件Tag
	 */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameplayEvent_Death);
	
	/*
	 * 在应用伤害性GE的时候,会根据原因向GE资产标签中附加对应的原因
	 */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_DamageSelfDestruct);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_FellOutOfWorld);

	/*
	 * 技能在失败时 将会通过CDO的配置,使用Tag向Message系统广播消息(附带文本/蒙太奇)
	 */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_SimpleTextFailure_Message)
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_PlayMontageFailure_Message)

	/*
	 * 技能在通过激活管线时 如果失败将会 向技能中添加失败的原因Tag
	 */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_ActivateFail_ActivationGroup)
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_ActivateFail_IsDead)

	/*
	 * 死亡流程使用的标识性Tag 代表死亡流程中的状态,和最终应用并且常驻的死亡Tag
	 */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Death)
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Death_Dying)
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Death_Dead)

	/*
	 * 技能使用的标识性Tag 豁免死亡开始和 Pawn 注销 ASC 时的批量取消，不影响技能授予或其他取消规则。
	 */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Behavior_AvoidDeathClear)
}

#undef UE_API
