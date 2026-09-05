// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Rules/TeamDamageRuleBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TeamDamageRuleBase)

ETeamDamageRuleResult UTeamDamageRuleBase::EvaluateDamage_Implementation(const FTeamDamageRuleContext& Context) const
{
	// 基类默认: 不表态(弃权), 由子类决定
	return ETeamDamageRuleResult::Continue;
}
