// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Net/Serialization/FastArraySerializer.h"
#include "Templates/SubclassOf.h"
#include "InventoryDataTypes.generated.h"

#define UE_API CUSTOMINVENTORY_API

class UActorComponent;
struct FInventoryList;
class UInventoryManager;
class UInventoryItemDefinition;
class UInventoryItemInstance;

/** 库存条目复制增删或数量变化时，通过消息路由发布的变更信息。 */
USTRUCT(BlueprintType, MinimalAPI)
struct FInventoryChangeMessage
{
	GENERATED_BODY()

	//@TODO: 基于标签的名称+拥有库存的演员，而不是直接暴露组件？ “直接存储组件指针”和“通过 Actor + 标签来查找组件”哪个更好。
	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	TObjectPtr<UActorComponent> InventoryOwner = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	TObjectPtr<UInventoryItemInstance> Instance = nullptr;

	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	int32 NewCount = 0;

	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	int32 Delta = 0;
};

/** 快速数组中的库存条目，记录物品实例及堆叠数量。 */
USTRUCT(BlueprintType, MinimalAPI)
struct FInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FInventoryEntry() { ; }

	FString GetDebugString() const;

private:
	friend FInventoryList;
	friend UInventoryManager;

	UPROPERTY()
	TObjectPtr<UInventoryItemInstance> Instance = nullptr;

	UPROPERTY()
	int32 StackCount = 0;

	UPROPERTY(NotReplicated)
	int32 LastObservedCount = INDEX_NONE;
};

/** 管理库存条目复制，并通过复制回调发布数量变化消息。 */
USTRUCT(BlueprintType, MinimalAPI)
struct FInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

	FInventoryList() : OwnerComponent(nullptr) { ; }

	FInventoryList(UActorComponent* InOwnerComponent) : OwnerComponent(InOwnerComponent) { ; }

	TArray<UInventoryItemInstance*> GetAllItems() const;

	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryEntry, FInventoryList>(Entries, DeltaParms, *this);
	}

	UInventoryItemInstance* AddEntry(TSubclassOf<UInventoryItemDefinition> ItemClass, int32 StackCount);
	/** 添加已有物品实例尚未实现，当前调用会触发 unimplemented()。 */
	void AddEntry(UInventoryItemInstance* Instance);

	void RemoveEntry(UInventoryItemInstance* Instance);

private:
	void BroadcastChangeMessage(FInventoryEntry& Entry, int32 OldCount, int32 NewCount);

	friend UInventoryManager;

	/** 通过快速数组复制的库存条目。 */
	UPROPERTY()
	TArray<FInventoryEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

template <>
struct TStructOpsTypeTraits<FInventoryList> : public TStructOpsTypeTraitsBase2<FInventoryList>
{
	enum { WithNetDeltaSerializer = true };
};

#undef UE_API
