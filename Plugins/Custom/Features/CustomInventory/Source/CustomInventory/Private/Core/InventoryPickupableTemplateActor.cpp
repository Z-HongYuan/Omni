// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Core/InventoryPickupableTemplateActor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryPickupableTemplateActor)

AInventoryPickupableTemplateActor::AInventoryPickupableTemplateActor()
{
}

FInventoryPickup AInventoryPickupableTemplateActor::GetPickupInventory() const
{
	return PickupInventory;
}
