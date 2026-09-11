// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Abilities/ExtAbilityCost_PlayerTagStack.h"
#include "Component/ExtTagsStackComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExtAbilityCost_PlayerTagStack)

UExtAbilityCost_PlayerTagStack::UExtAbilityCost_PlayerTagStack()
{
	Quantity.SetValue(1.0f);
}

bool UExtAbilityCost_PlayerTagStack::CheckCost(const UExtGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (AController* PC = Ability->GetControllerFromActorInfo())
	{
		//查找组件,有没有UExtTagsStackComponent
		if (UExtTagsStackComponent* TagsStackComponent = PC->PlayerState->GetComponentByClass<UExtTagsStackComponent>())
		{
			const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

			const float NumStacksReal = Quantity.GetValueAtLevel(AbilityLevel);
			const int32 NumStacks = FMath::TruncToInt(NumStacksReal);

			return TagsStackComponent->GetStatTagStackCount(Tag) >= NumStacks;
		}
	}
	return false;
}

void UExtAbilityCost_PlayerTagStack::ApplyCost(const UExtGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo->IsNetAuthority())
	{
		if (AController* PC = Ability->GetControllerFromActorInfo())
		{
			if (UExtTagsStackComponent* TagsStackComponent = PC->PlayerState->GetComponentByClass<UExtTagsStackComponent>())
			{
				const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

				const float NumStacksReal = Quantity.GetValueAtLevel(AbilityLevel);
				const int32 NumStacks = FMath::TruncToInt(NumStacksReal);

				TagsStackComponent->RemoveStatTagStack(Tag, NumStacks);
			}
		}
	}
}
