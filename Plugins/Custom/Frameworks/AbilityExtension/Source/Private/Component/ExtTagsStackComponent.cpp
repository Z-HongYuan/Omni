// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Component/ExtTagsStackComponent.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExtTagsStackComponent)

UExtTagsStackComponent::UExtTagsStackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UExtTagsStackComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UExtTagsStackComponent, GameplayTagStackContainer);
}

void UExtTagsStackComponent::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	GameplayTagStackContainer.AddStack(Tag, StackCount);
}

int32 UExtTagsStackComponent::GetStatTagStackCount(FGameplayTag Tag) const
{
	return GameplayTagStackContainer.GetStackCount(Tag);
}

void UExtTagsStackComponent::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	GameplayTagStackContainer.RemoveStack(Tag, StackCount);
}

bool UExtTagsStackComponent::HasStatTag(FGameplayTag Tag) const
{
	return GameplayTagStackContainer.ContainsTag(Tag);
}
