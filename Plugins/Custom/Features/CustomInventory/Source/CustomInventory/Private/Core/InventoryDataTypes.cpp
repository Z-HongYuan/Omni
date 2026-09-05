// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Core/InventoryDataTypes.h"

#include "InventoryTags.h"
#include "MessageRouterManager.h"
#include "Components/ActorComponent.h"
#include "Core/InventoryItemDefinition.h"
#include "Core/InventoryItemInstance.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "ItemFragment/InventoryItemFragmentBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryDataTypes)

FString FInventoryEntry::GetDebugString() const
{
	TSubclassOf<UInventoryItemDefinition> ItemDef;
	if (Instance != nullptr)
	{
		ItemDef = Instance->GetItemDef();
	}

	return FString::Printf(TEXT("%s (%d x %s)"), *GetNameSafe(Instance), StackCount, *GetNameSafe(ItemDef));
}

TArray<UInventoryItemInstance*> FInventoryList::GetAllItems() const
{
	TArray<UInventoryItemInstance*> Results;
	Results.Reserve(Entries.Num());
	for (const FInventoryEntry& Entry : Entries)
	{
		if (Entry.Instance != nullptr) //@TODO: 更愿意不在这里处理这些，更想隐藏它？ 这里最好做一个错误处理,例如移除这个库存实例,选择报错或者其他补救措施
		{
			Results.Add(Entry.Instance);
		}
	}
	return Results;
}

UInventoryItemInstance* FInventoryList::AddEntry(TSubclassOf<UInventoryItemDefinition> ItemClass, int32 StackCount)
{
	UInventoryItemInstance* Result = nullptr;

	// 空指针检查
	check(ItemClass != nullptr);
	check(OwnerComponent);

	// 检查联网权限
	AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor->HasAuthority());

	// 添加一个默认构造,并且对其进行修改
	FInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Instance = NewObject<UInventoryItemInstance>(OwnerComponent->GetOwner()); //@TODO: 由于 UE-127172，外部使用actor而非组件
	NewEntry.Instance->SetItemDef(ItemClass);
	for (UInventoryItemFragmentBase* Fragment : GetDefault<UInventoryItemDefinition>(ItemClass)->Fragments)
	{
		if (Fragment != nullptr)
		{
			Fragment->OnInstanceCreated(NewEntry.Instance);
		}
	}
	NewEntry.StackCount = StackCount;
	Result = NewEntry.Instance;

	//const UInventoryItemDefinition* ItemCDO = GetDefault<UInventoryItemDefinition>(ItemDef);
	MarkItemDirty(NewEntry);

	return Result;
}

void FInventoryList::AddEntry(UInventoryItemInstance* Instance)
{
	unimplemented();
}

void FInventoryList::RemoveEntry(UInventoryItemInstance* Instance)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FInventoryEntry& Entry = *EntryIt;
		if (Entry.Instance == Instance)
		{
			EntryIt.RemoveCurrent();
			MarkArrayDirty();
		}
	}
}

void FInventoryList::BroadcastChangeMessage(FInventoryEntry& Entry, int32 OldCount, int32 NewCount)
{
	FInventoryChangeMessage Message;
	Message.InventoryOwner = OwnerComponent;
	Message.Instance = Entry.Instance;
	Message.NewCount = NewCount;
	Message.Delta = NewCount - OldCount;

	UMessageRouterManager& MessageSystem = UMessageRouterManager::Get(OwnerComponent->GetWorld());
	MessageSystem.BroadcastMessage(InventoryTags::TAG_Inventory_Message_StackChanged, Message);
}

void FInventoryList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		FInventoryEntry& Stack = Entries[Index];
		BroadcastChangeMessage(Stack, Stack.StackCount, 0);
		Stack.LastObservedCount = 0;
	}
}

void FInventoryList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FInventoryEntry& Stack = Entries[Index];
		BroadcastChangeMessage(Stack, 0, Stack.StackCount);
		Stack.LastObservedCount = Stack.StackCount;
	}
}

void FInventoryList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FInventoryEntry& Stack = Entries[Index];
		check(Stack.LastObservedCount != INDEX_NONE);
		BroadcastChangeMessage(Stack, Stack.LastObservedCount, Stack.StackCount);
		Stack.LastObservedCount = Stack.StackCount;
	}
}
