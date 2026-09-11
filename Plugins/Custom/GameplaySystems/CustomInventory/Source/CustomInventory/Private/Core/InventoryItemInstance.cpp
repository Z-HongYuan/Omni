// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Core/InventoryItemInstance.h"

#include "Core/InventoryItemDefinition.h"
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#include "ItemFragment/InventoryItemFragmentBase.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryItemInstance)

UInventoryItemInstance::UInventoryItemInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UInventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, StatTags);
	DOREPLIFETIME(ThisClass, ItemDef);
}

void UInventoryItemInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	using namespace UE::Net;

	// 构建描述符并为该对象分配PropertyReplicationFragments
	FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}

void UInventoryItemInstance::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.AddStack(Tag, StackCount);
}

void UInventoryItemInstance::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.RemoveStack(Tag, StackCount);
}

int32 UInventoryItemInstance::GetStatTagStackCount(FGameplayTag Tag) const
{
	return StatTags.GetStackCount(Tag);
}

bool UInventoryItemInstance::HasStatTag(FGameplayTag Tag) const
{
	return StatTags.ContainsTag(Tag);
}

const UInventoryItemFragmentBase* UInventoryItemInstance::FindFragmentByClass(TSubclassOf<UInventoryItemFragmentBase> FragmentClass) const
{
	if (ItemDef == nullptr || FragmentClass == nullptr) return nullptr;

	return GetDefault<UInventoryItemDefinition>(ItemDef)->FindFragmentByClass(FragmentClass);
}

void UInventoryItemInstance::SetItemDef(TSubclassOf<UInventoryItemDefinition> InDef)
{
	ItemDef = InDef;
}
