// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Core/InventoryItemDefinition.h"
#include "ItemFragment/InventoryItemFragmentBase.h"
#include "Templates/SubclassOf.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryItemDefinition)

UInventoryItemDefinition::UInventoryItemDefinition(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

const UInventoryItemFragmentBase* UInventoryItemDefinition::FindFragmentByClass(TSubclassOf<UInventoryItemFragmentBase> FragmentClass) const
{
	if (FragmentClass == nullptr) return nullptr;

	for (const auto& Fragment : Fragments)
	{
		if (Fragment && Fragment->IsA(FragmentClass))
			return Fragment;
	}

	return nullptr;
}
