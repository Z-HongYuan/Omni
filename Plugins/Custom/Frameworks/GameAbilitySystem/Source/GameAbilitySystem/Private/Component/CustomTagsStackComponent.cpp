// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Component/CustomTagsStackComponent.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CustomTagsStackComponent)

UCustomTagsStackComponent::UCustomTagsStackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UCustomTagsStackComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCustomTagsStackComponent, GameplayTagStackContainer);
}

void UCustomTagsStackComponent::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	GameplayTagStackContainer.AddStack(Tag, StackCount);
}

int32 UCustomTagsStackComponent::GetStatTagStackCount(FGameplayTag Tag) const
{
	return GameplayTagStackContainer.GetStackCount(Tag);
}

void UCustomTagsStackComponent::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	GameplayTagStackContainer.RemoveStack(Tag, StackCount);
}

bool UCustomTagsStackComponent::HasStatTag(FGameplayTag Tag) const
{
	return GameplayTagStackContainer.ContainsTag(Tag);
}
