// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Core/QuickBarComponent.h"

#include "CustomEquipmentTags.h"
#include "MessageRouterManager.h"
#include "NativeGameplayTags.h"
#include "Actions/InventoryFragment_EquippableItem.h"
#include "Core/EquipmentDefinition.h"
#include "Core/EquipmentInstance.h"
#include "Core/EquipmentManager.h"
#include "Core/InventoryItemInstance.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(QuickBarComponent)

UQuickBarComponent::UQuickBarComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UQuickBarComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Slots);
	DOREPLIFETIME(ThisClass, ActiveSlotIndex);
}

void UQuickBarComponent::CycleActiveSlotForward()
{
	if (Slots.Num() < 2)
	{
		return;
	}

	const int32 OldIndex = (ActiveSlotIndex < 0 ? Slots.Num() - 1 : ActiveSlotIndex);
	int32 NewIndex = ActiveSlotIndex;
	do
	{
		NewIndex = (NewIndex + 1) % Slots.Num();
		if (Slots[NewIndex] != nullptr)
		{
			SetActiveSlotIndex(NewIndex);
			return;
		}
	}
	while (NewIndex != OldIndex);
}

void UQuickBarComponent::CycleActiveSlotBackward()
{
	if (Slots.Num() < 2)
	{
		return;
	}

	const int32 OldIndex = (ActiveSlotIndex < 0 ? Slots.Num() - 1 : ActiveSlotIndex);
	int32 NewIndex = ActiveSlotIndex;
	do
	{
		NewIndex = (NewIndex - 1 + Slots.Num()) % Slots.Num();
		if (Slots[NewIndex] != nullptr)
		{
			SetActiveSlotIndex(NewIndex);
			return;
		}
	}
	while (NewIndex != OldIndex);
}

UInventoryItemInstance* UQuickBarComponent::GetActiveSlotItem() const
{
	return Slots.IsValidIndex(ActiveSlotIndex) ? Slots[ActiveSlotIndex] : nullptr;
}

int32 UQuickBarComponent::GetNextFreeItemSlot() const
{
	int32 SlotIndex = 0;
	for (const TObjectPtr<UInventoryItemInstance>& ItemPtr : Slots)
	{
		if (ItemPtr == nullptr)
		{
			return SlotIndex;
		}
		++SlotIndex;
	}

	return INDEX_NONE;
}

void UQuickBarComponent::AddItemToSlot(int32 SlotIndex, UInventoryItemInstance* Item)
{
	if (Slots.IsValidIndex(SlotIndex) && (Item != nullptr))
	{
		if (Slots[SlotIndex] == nullptr)
		{
			Slots[SlotIndex] = Item;
			OnRep_Slots();
		}
	}
}

UInventoryItemInstance* UQuickBarComponent::RemoveItemFromSlot(int32 SlotIndex)
{
	UInventoryItemInstance* Result = nullptr;

	if (ActiveSlotIndex == SlotIndex)
	{
		UnequipItemInSlot();
		ActiveSlotIndex = -1;
	}

	if (Slots.IsValidIndex(SlotIndex))
	{
		Result = Slots[SlotIndex];

		if (Result != nullptr)
		{
			Slots[SlotIndex] = nullptr;
			OnRep_Slots();
		}
	}

	return Result;
}

void UQuickBarComponent::BeginPlay()
{
	if (Slots.Num() < NumSlots)
	{
		Slots.AddDefaulted(NumSlots - Slots.Num());
	}
	Super::BeginPlay();
}

void UQuickBarComponent::UnequipItemInSlot()
{
	if (UEquipmentManager* EquipmentManager = FindEquipmentManager())
	{
		if (EquippedItem != nullptr)
		{
			EquipmentManager->UnequipItem(EquippedItem);
			EquippedItem = nullptr;
		}
	}
}

void UQuickBarComponent::EquipItemInSlot()
{
	check(Slots.IsValidIndex(ActiveSlotIndex));
	check(EquippedItem == nullptr);

	if (UInventoryItemInstance* SlotItem = Slots[ActiveSlotIndex])
	{
		if (const UInventoryFragment_EquippableItem* EquipInfo = Cast<UInventoryFragment_EquippableItem>(SlotItem->FindFragmentByClass(UInventoryFragment_EquippableItem::StaticClass())))
		{
			TSubclassOf<UEquipmentDefinition> EquipDef = EquipInfo->EquipmentDefinition;
			if (EquipDef != nullptr)
			{
				if (UEquipmentManager* EquipmentManager = FindEquipmentManager())
				{
					EquippedItem = EquipmentManager->EquipItem(EquipDef);
					if (EquippedItem != nullptr)
					{
						EquippedItem->SetInstigator(SlotItem);
					}
				}
			}
		}
	}
}

UEquipmentManager* UQuickBarComponent::FindEquipmentManager() const
{
	if (AController* OwnerController = Cast<AController>(GetOwner()))
	{
		if (APawn* Pawn = OwnerController->GetPawn())
		{
			return Pawn->FindComponentByClass<UEquipmentManager>();
		}
	}
	return nullptr;
}

void UQuickBarComponent::OnRep_Slots()
{
	FQuickBarSlotsChangedMessage Message;
	Message.Owner = GetOwner();
	Message.Slots = Slots;

	UMessageRouterManager& MessageSystem = UMessageRouterManager::Get(this);
	MessageSystem.BroadcastMessage(CustomEquipmentTags::TAG_QuickBar_Message_SlotsChanged, Message);
}

void UQuickBarComponent::OnRep_ActiveSlotIndex()
{
	FQuickBarActiveIndexChangedMessage Message;
	Message.Owner = GetOwner();
	Message.ActiveIndex = ActiveSlotIndex;

	UMessageRouterManager& MessageSystem = UMessageRouterManager::Get(this);
	MessageSystem.BroadcastMessage(CustomEquipmentTags::TAG_QuickBar_Message_ActiveIndexChanged, Message);
}

void UQuickBarComponent::SetActiveSlotIndex_Implementation(int32 NewIndex)
{
	if (Slots.IsValidIndex(NewIndex) && (ActiveSlotIndex != NewIndex))
	{
		UnequipItemInSlot();

		ActiveSlotIndex = NewIndex;

		EquipItemInSlot();

		OnRep_ActiveSlotIndex();
	}
}
