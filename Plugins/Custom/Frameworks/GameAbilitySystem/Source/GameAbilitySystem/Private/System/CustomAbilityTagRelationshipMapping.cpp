// Copyright © 2026 张鸿源. All Rights Reserved.


#include "System/CustomAbilityTagRelationshipMapping.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CustomAbilityTagRelationshipMapping)

void UCustomAbilityTagRelationshipMapping::GetAbilityTagsToBlockAndCancel(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutTagsToBlock, FGameplayTagContainer* OutTagsToCancel) const
{
	// 目前先做个简单的迭代
	for (const FCustomAbilityTagRelationship& Tags : AbilityTagRelationships)
	{
		if (AbilityTags.HasTag(Tags.AbilityTag))
		{
			if (OutTagsToBlock)
			{
				OutTagsToBlock->AppendTags(Tags.AbilityTagsToBlock);
			}
			if (OutTagsToCancel)
			{
				OutTagsToCancel->AppendTags(Tags.AbilityTagsToCancel);
			}
		}
	}
}

void UCustomAbilityTagRelationshipMapping::GetRequiredAndBlockedActivationTags(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutActivationRequired, FGameplayTagContainer* OutActivationBlocked) const
{
	// 目前先做个简单的迭代
	for (const FCustomAbilityTagRelationship& Tags : AbilityTagRelationships)
	{
		if (AbilityTags.HasTag(Tags.AbilityTag))
		{
			if (OutActivationRequired)
			{
				OutActivationRequired->AppendTags(Tags.ActivationRequiredTags);
			}
			if (OutActivationBlocked)
			{
				OutActivationBlocked->AppendTags(Tags.ActivationBlockedTags);
			}
		}
	}
}

bool UCustomAbilityTagRelationshipMapping::IsAbilityCancelledByTag(const FGameplayTagContainer& AbilityTags, const FGameplayTag& ActionTag) const
{
	// 目前先做个简单的迭代
	for (const FCustomAbilityTagRelationship& Tags : AbilityTagRelationships)
	{
		if (Tags.AbilityTag == ActionTag && Tags.AbilityTagsToCancel.HasAny(AbilityTags))
		{
			return true;
		}
	}
	return false;
}
