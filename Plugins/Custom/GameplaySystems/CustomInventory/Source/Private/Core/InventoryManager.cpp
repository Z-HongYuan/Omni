// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Core/InventoryManager.h"

#include "Core/InventoryItemDefinition.h"
#include "Core/InventoryItemInstance.h"
#include "Engine/ActorChannel.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryManager)

UInventoryManager::UInventoryManager(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer), InventoryList(this)
{
	SetIsReplicatedByDefault(true);
}

void UInventoryManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, InventoryList);
}

bool UInventoryManager::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (FInventoryEntry& Entry : InventoryList.Entries)
	{
		UInventoryItemInstance* Instance = Entry.Instance;

		if (Instance && IsValid(Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}

void UInventoryManager::ReadyForReplication()
{
	Super::ReadyForReplication();

	// 复制就绪后，补注册此前创建的物品实例。
	if (IsUsingRegisteredSubObjectList())
	{
		for (const FInventoryEntry& Entry : InventoryList.Entries)
		{
			UInventoryItemInstance* Instance = Entry.Instance;

			if (IsValid(Instance))
			{
				AddReplicatedSubObject(Instance);
			}
		}
	}
}

bool UInventoryManager::CanAddItemDefinition(TSubclassOf<UInventoryItemDefinition> ItemDef, int32 StackCount)
{
	// TODO: 补充堆叠上限、唯一性及库存容量等检查。
	return true;
}

UInventoryItemInstance* UInventoryManager::AddItemDefinition(TSubclassOf<UInventoryItemDefinition> ItemDef, int32 StackCount)
{
	UInventoryItemInstance* Result = nullptr;
	if (ItemDef != nullptr)
	{
		// 由库存列表创建实例并执行属性片段初始化。
		Result = InventoryList.AddEntry(ItemDef, StackCount);

		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && Result)
		{
			AddReplicatedSubObject(Result);
		}
	}
	return Result;
}

void UInventoryManager::AddItemInstance(UInventoryItemInstance* ItemInstance)
{
	InventoryList.AddEntry(ItemInstance);
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && ItemInstance)
	{
		AddReplicatedSubObject(ItemInstance);
	}
}

void UInventoryManager::RemoveItemInstance(UInventoryItemInstance* ItemInstance)
{
	InventoryList.RemoveEntry(ItemInstance);

	if (ItemInstance && IsUsingRegisteredSubObjectList())
	{
		RemoveReplicatedSubObject(ItemInstance);
	}
}

TArray<UInventoryItemInstance*> UInventoryManager::GetAllItems() const
{
	return InventoryList.GetAllItems();
}

UInventoryItemInstance* UInventoryManager::FindFirstItemStackByDefinition(TSubclassOf<UInventoryItemDefinition> ItemDef) const
{
	for (const FInventoryEntry& Entry : InventoryList.Entries)
	{
		UInventoryItemInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
		{
			if (Instance->GetItemDef() == ItemDef)
			{
				return Instance;
			}
		}
	}

	return nullptr;
}

int32 UInventoryManager::GetTotalItemCountByDefinition(TSubclassOf<UInventoryItemDefinition> ItemDef) const
{
	int32 TotalCount = 0;
	for (const FInventoryEntry& Entry : InventoryList.Entries)
	{
		UInventoryItemInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
		{
			if (Instance->GetItemDef() == ItemDef)
			{
				++TotalCount;
			}
		}
	}

	return TotalCount;
}

int32 UInventoryManager::GetItemCount(UInventoryItemInstance* ItemInstance) const
{
	for (const FInventoryEntry& Entry : InventoryList.Entries)
	{
		UInventoryItemInstance* Instance = Entry.Instance;

		if (IsValid(Instance) && Instance == ItemInstance)
		{
			return Entry.StackCount;
		}
	}
	return 0;
}

bool UInventoryManager::ConsumeItemsByDefinition(TSubclassOf<UInventoryItemDefinition> ItemDef, int32 NumToConsume)
{
	AActor* OwningActor = GetOwner();
	if (!OwningActor || !OwningActor->HasAuthority())
	{
		return false;
	}

	//@TODO: 现在是n平方，因为没有加速结构 , 可以优化一下,使用TMap之类的加速数据结构
	int32 TotalConsumed = 0;
	while (TotalConsumed < NumToConsume)
	{
		if (UInventoryItemInstance* Instance = FindFirstItemStackByDefinition(ItemDef))
		{
			InventoryList.RemoveEntry(Instance);
			++TotalConsumed;
		}
		else
		{
			return false;
		}
	}

	return TotalConsumed == NumToConsume;
}
