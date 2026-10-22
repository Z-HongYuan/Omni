// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"
#include "UObject/Interface.h"
#include "InventoryPickupableInterface.generated.h"

#define  UE_API CUSTOMINVENTORY_API

class UInventoryManager;
class UInventoryItemInstance;
class UInventoryItemDefinition;

/** 拾取时用于创建库存条目的物品定义与数量。 */
USTRUCT(BlueprintType)
struct FPickupTemplate
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	int32 StackCount = 1;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UInventoryItemDefinition> ItemDef;
};

/** 拾取内容中引用的已有物品实例。 */
USTRUCT(BlueprintType)
struct FPickupInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UInventoryItemInstance> Item = nullptr;
};

USTRUCT(BlueprintType)
struct FInventoryPickup
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FPickupInstance> Instances;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FPickupTemplate> Templates;
};

/** 提供可加入库存的物品定义和已有实例。 */
UINTERFACE(MinimalAPI, BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UInventoryPickupableInterface : public UInterface
{
	GENERATED_BODY()
};

class IInventoryPickupableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual FInventoryPickup GetPickupInventory() const = 0;
};

UCLASS()
class UPickupableStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UPickupableStatics();

	/** 优先查询 Actor 自身，再返回首个实现拾取接口的组件。 */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	static TScriptInterface<IInventoryPickupableInterface> GetFirstPickupableFromActor(AActor* Actor);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, meta = (WorldContext = "Ability"), Category = "Inventory")
	static void AddPickupToInventory(UInventoryManager* InventoryComponent, TScriptInterface<IInventoryPickupableInterface> Pickup);
};

#undef UE_API
