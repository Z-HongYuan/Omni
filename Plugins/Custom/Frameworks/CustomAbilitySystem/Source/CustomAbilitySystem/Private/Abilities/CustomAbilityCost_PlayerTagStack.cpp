// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Abilities/CustomAbilityCost_PlayerTagStack.h"
#include "Component/CustomTagsStackComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CustomAbilityCost_PlayerTagStack)

UCustomAbilityCost_PlayerTagStack::UCustomAbilityCost_PlayerTagStack()
{
	Quantity.SetValue(1.0f);
}

bool UCustomAbilityCost_PlayerTagStack::CheckCost(const UCustomGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (AController* PC = Ability->GetControllerFromActorInfo())
	{
		//查找组件,有没有UCustomTagsStackComponent
		if (UCustomTagsStackComponent* TagsStackComponent = PC->PlayerState->GetComponentByClass<UCustomTagsStackComponent>())
		{
			const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

			const float NumStacksReal = Quantity.GetValueAtLevel(AbilityLevel);
			const int32 NumStacks = FMath::TruncToInt(NumStacksReal);

			return TagsStackComponent->GetStatTagStackCount(Tag) >= NumStacks;
		}
	}
	return false;
}

void UCustomAbilityCost_PlayerTagStack::ApplyCost(const UCustomGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo->IsNetAuthority())
	{
		if (AController* PC = Ability->GetControllerFromActorInfo())
		{
			if (UCustomTagsStackComponent* TagsStackComponent = PC->PlayerState->GetComponentByClass<UCustomTagsStackComponent>())
			{
				const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

				const float NumStacksReal = Quantity.GetValueAtLevel(AbilityLevel);
				const int32 NumStacks = FMath::TruncToInt(NumStacksReal);

				TagsStackComponent->RemoveStatTagStack(Tag, NumStacks);
			}
		}
	}
}
