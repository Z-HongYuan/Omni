// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Rules/TeamDamageRuleBase.h"
#include "TeamDamageDefaultRules.generated.h"

#define UE_API CUSTOMTEAM_API

/**
 * 自伤规则：上下文的 bSamePlayer 为 true 时允许伤害，其余情况弃权。
 * 仅在 bUseDefaultRules 开启时由子系统初始化注册。
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
 * 队伍关系规则：不同队伍允许伤害，同队伍禁止伤害，归属无效时弃权。
 * bSamePlayer 为 true 时弃权，交由自伤规则及其他规则处理。
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
