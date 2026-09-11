// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "InventoryDataTypes.h"
#include "Components/ActorComponent.h"
#include "InventoryManager.generated.h"

#define UE_API CUSTOMINVENTORY_API

class UInventoryItemDefinition;

/*
 * 库存管理器
 */
UCLASS(MinimalAPI, BlueprintType, Meta=(BlueprintSpawnableComponent))
class UInventoryManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UE_API UInventoryManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UObject interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UE_API virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	UE_API virtual void ReadyForReplication() override;
	//~End of UObject interface

	// 是否能添加物品
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	UE_API bool CanAddItemDefinition(TSubclassOf<UInventoryItemDefinition> ItemDef, int32 StackCount = 1);

	// 向List添加一个物品(通过定义)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	UE_API UInventoryItemInstance* AddItemDefinition(TSubclassOf<UInventoryItemDefinition> ItemDef, int32 StackCount = 1);

	// 向List添加一个物品(实例) (List内未实现)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	UE_API void AddItemInstance(UInventoryItemInstance* ItemInstance);

	// 移除物品(实例)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	UE_API void RemoveItemInstance(UInventoryItemInstance* ItemInstance);

	// 获取所有物品(实例)
	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure=false)
	UE_API TArray<UInventoryItemInstance*> GetAllItems() const;

	// 通过物品定义查找第一个物品实例
	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure)
	UE_API UInventoryItemInstance* FindFirstItemStackByDefinition(TSubclassOf<UInventoryItemDefinition> ItemDef) const;

	// 获取指定物品定义在库存中的数量
	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure)
	UE_API int32 GetTotalItemCountByDefinition(TSubclassOf<UInventoryItemDefinition> ItemDef) const;

	// 获取指定物品实例的数量
	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure)
	UE_API int32 GetItemCount(UInventoryItemInstance* ItemInstance) const;

	// 通过物品定义消耗物品,返回是否消费成功,false表示库存内没有该物品,或者剩余数量不足
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	UE_API bool ConsumeItemsByDefinition(TSubclassOf<UInventoryItemDefinition> ItemDef, int32 NumToConsume);

private:
	UPROPERTY(Replicated)
	FInventoryList InventoryList;
};

#undef UE_API
