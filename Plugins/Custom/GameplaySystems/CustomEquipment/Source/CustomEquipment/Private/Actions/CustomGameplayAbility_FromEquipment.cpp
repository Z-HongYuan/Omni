// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Actions/CustomGameplayAbility_FromEquipment.h"
#include "Core/EquipmentInstance.h"
#include "Core/InventoryItemInstance.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(CustomGameplayAbility_FromEquipment)

UCustomGameplayAbility_FromEquipment::UCustomGameplayAbility_FromEquipment(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

UEquipmentInstance* UCustomGameplayAbility_FromEquipment::GetAssociatedEquipment() const
{
	if (FGameplayAbilitySpec* Spec = UGameplayAbility::GetCurrentAbilitySpec())
	{
		return Cast<UEquipmentInstance>(Spec->SourceObject.Get());
	}

	return nullptr;
}

UInventoryItemInstance* UCustomGameplayAbility_FromEquipment::GetAssociatedItem() const
{
	if (UEquipmentInstance* Equipment = GetAssociatedEquipment())
	{
		return Cast<UInventoryItemInstance>(Equipment->GetInstigator());
	}
	return nullptr;
}

#if WITH_EDITOR
EDataValidationResult UCustomGameplayAbility_FromEquipment::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	if (InstancingPolicy == EGameplayAbilityInstancingPolicy::NonInstanced)
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	{
		Context.AddError(NSLOCTEXT("UCustomGameplayAbility_FromEquipment", "EquipmentAbilityMustBeInstanced", "Equipment ability must be instanced"));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif
