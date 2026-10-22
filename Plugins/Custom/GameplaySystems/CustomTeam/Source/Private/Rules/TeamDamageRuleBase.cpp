// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Rules/TeamDamageRuleBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TeamDamageRuleBase)

ETeamDamageRuleResult UTeamDamageRuleBase::EvaluateDamage_Implementation(const FTeamDamageRuleContext& Context) const
{
	// 基类默认弃权，具体策略由子类提供。
	return ETeamDamageRuleResult::Continue;
}
