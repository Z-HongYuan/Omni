// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Rules/TeamDamageDefaultRules.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TeamDamageDefaultRules)

ETeamDamageRuleResult UTeamDamageRule_SelfDamage::EvaluateDamage_Implementation(const FTeamDamageRuleContext& Context) const
{
	return Context.bSamePlayer ? ETeamDamageRuleResult::Allow : ETeamDamageRuleResult::Continue;
}

ETeamDamageRuleResult UTeamDamageRule_TeamRelationship::EvaluateDamage_Implementation(const FTeamDamageRuleContext& Context) const
{
	// 同一玩家: 由自我豁免规则处理, 这里不表态(否则会被队伍关系的 Block 误伤)
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
		// 友军误伤默认关闭
		return ETeamDamageRuleResult::Block;
	}

	// 无队伍归属(InvalidArgument): 弃权, 交给项目注册的规则或默认禁止
	return ETeamDamageRuleResult::Continue;
}
