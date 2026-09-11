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

/*
 * 提供能够拾取的模版结构体
 */
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

/*
 * 提供拾取实例结构体
 */
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

/*
 * 提供拾取的接口
 */
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

	UFUNCTION(BlueprintPure, Category = "Inventory")
	static TScriptInterface<IInventoryPickupableInterface> GetFirstPickupableFromActor(AActor* Actor);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, meta = (WorldContext = "Ability"), Category = "Inventory")
	static void AddPickupToInventory(UInventoryManager* InventoryComponent, TScriptInterface<IInventoryPickupableInterface> Pickup);
};

#undef UE_API
