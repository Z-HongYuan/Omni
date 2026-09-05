// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ItemFragment/InventoryItemFragmentBase.h"
#include "Templates/SubclassOf.h"
#include "InventoryFragment_EquippableItem.generated.h"

#define UE_API CUSTOMEQUIPMENT_API

class UEquipmentDefinition;

/**
 * 
 */
UCLASS(MinimalAPI)
class UInventoryFragment_EquippableItem : public UInventoryItemFragmentBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category=Inventory)
	TSubclassOf<UEquipmentDefinition> EquipmentDefinition;
};

#undef UE_API
