// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Rules/TeamDamageDefaultRules.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TeamDamageDefaultRules)

ETeamDamageRuleResult UTeamDamageRule_SelfDamage::EvaluateDamage_Implementation(const FTeamDamageRuleContext& Context) const
{
	return Context.bSamePlayer ? ETeamDamageRuleResult::Allow : ETeamDamageRuleResult::Continue;
}

ETeamDamageRuleResult UTeamDamageRule_TeamRelationship::EvaluateDamage_Implementation(const FTeamDamageRuleContext& Context) const
{
	// 同一玩家由自伤及其他规则处理，避免队伍规则直接否决。
	if (Context.bSamePlayer)
	{
		return ETeamDamageRuleResult::Continue;
	}

	if (Context.Relationship == ETeamComparison::DifferentTeams)
	{
		return ETeamDamageRuleResult::Allow;
	}

	if (Context.Relationship == ETeamComparison::OnSameTeam)
	{
		// 同队伍伤害由此规则直接否决。
		return ETeamDamageRuleResult::Block;
	}

	// 队伍归属无效时弃权，交给其他规则和默认伤害策略。
	return ETeamDamageRuleResult::Continue;
}
