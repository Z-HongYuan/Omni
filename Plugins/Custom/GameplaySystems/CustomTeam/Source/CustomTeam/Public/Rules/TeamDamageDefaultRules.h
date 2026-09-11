// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Rules/TeamDamageRuleBase.h"
#include "TeamDamageDefaultRules.generated.h"

#define UE_API CUSTOMTEAM_API

/**
 * 内置默认规则: 自我豁免
 * 发起者与目标属于同一玩家(或同一对象)时 → Allow(允许自伤)。
 * 由 UTeamSubsystem::Initialize 默认注册; 项目可通过 ClearDamageRules 移除或禁用。
 */
UCLASS(MinimalAPI)
class UTeamDamageRule_SelfDamage : public UTeamDamageRuleBase
{
	GENERATED_BODY()

public:
	UTeamDamageRule_SelfDamage()
	{
		bEnabled = true;
		RuleName = FText::FromString("Self Damage");
	}

	UE_API virtual ETeamDamageRuleResult EvaluateDamage_Implementation(const FTeamDamageRuleContext& Context) const override;
};

/**
 * 内置默认规则: 队伍关系
 * 不同队伍 → Allow; 同队伍 → Block(友军误伤默认关闭); 无队伍归属 → 弃权。
 * 同一玩家场景不表态(交给自我豁免规则处理)。
 */
UCLASS(MinimalAPI)
class UTeamDamageRule_TeamRelationship : public UTeamDamageRuleBase
{
	GENERATED_BODY()

public:
	UTeamDamageRule_TeamRelationship()
	{
		bEnabled = true;
		RuleName = FText::FromString("Team Relationship");
	}

	UE_API virtual ETeamDamageRuleResult EvaluateDamage_Implementation(const FTeamDamageRuleContext& Context) const override;
};

#undef UE_API
