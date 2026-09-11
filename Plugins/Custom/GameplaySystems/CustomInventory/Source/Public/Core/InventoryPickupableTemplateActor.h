// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "InventoryPickupableInterface.h"
#include "GameFramework/Actor.h"
#include "InventoryPickupableTemplateActor.generated.h"

#define UE_API CUSTOMINVENTORY_API

UCLASS(MinimalAPI)
class AInventoryPickupableTemplateActor : public AActor, public IInventoryPickupableInterface
{
	GENERATED_BODY()

public:
	AInventoryPickupableTemplateActor();

	virtual FInventoryPickup GetPickupInventory() const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FInventoryPickup PickupInventory;
};
#undef UE_API
