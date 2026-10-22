// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "InventoryDataTypes.h"
#include "Components/ActorComponent.h"
#include "InventoryManager.generated.h"

#define UE_API CUSTOMINVENTORY_API

class UInventoryItemDefinition;

/** 管理库存条目、物品实例及其网络复制。 */
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

	/** 检查是否允许添加物品；当前始终返回 true，容量和堆叠限制尚未实现。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	UE_API bool CanAddItemDefinition(TSubclassOf<UInventoryItemDefinition> ItemDef, int32 StackCount = 1);

	/** 由权威端根据物品定义创建库存条目，并注册物品子对象复制。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	UE_API UInventoryItemInstance* AddItemDefinition(TSubclassOf<UInventoryItemDefinition> ItemDef, int32 StackCount = 1);

	/** 添加已有物品实例尚未实现，底层调用会触发 unimplemented()。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	UE_API void AddItemInstance(UInventoryItemInstance* ItemInstance);

	/** 移除指定物品实例，并取消其子对象复制注册。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	UE_API void RemoveItemInstance(UInventoryItemInstance* ItemInstance);

	/** 返回库存中所有非空物品实例。 */
	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure=false)
	UE_API TArray<UInventoryItemInstance*> GetAllItems() const;

	/** 返回首个使用指定定义的物品实例；没有匹配项时返回 nullptr。 */
	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure)
	UE_API UInventoryItemInstance* FindFirstItemStackByDefinition(TSubclassOf<UInventoryItemDefinition> ItemDef) const;

	/** 返回匹配物品定义的条目数，不累计各条目的 StackCount。 */
	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure)
	UE_API int32 GetTotalItemCountByDefinition(TSubclassOf<UInventoryItemDefinition> ItemDef) const;

	/** 返回指定物品实例所在条目的堆叠数量；没有匹配项时返回 0。 */
	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure)
	UE_API int32 GetItemCount(UInventoryItemInstance* ItemInstance) const;

	/**
	 * 由权威端按条目移除指定定义的物品，每个条目计为一次消耗。
	 * 权限不足或条目不足时返回 false；已移除的条目不会回滚。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	UE_API bool ConsumeItemsByDefinition(TSubclassOf<UInventoryItemDefinition> ItemDef, int32 NumToConsume);

private:
	UPROPERTY(Replicated)
	FInventoryList InventoryList;
};

#undef UE_API
