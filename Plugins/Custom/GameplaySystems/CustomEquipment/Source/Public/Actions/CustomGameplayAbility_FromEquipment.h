// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Abilities/ExtGameplayAbility.h"
#include "CustomGameplayAbility_FromEquipment.generated.h"

#define UE_API CUSTOMEQUIPMENT_API

class UInventoryItemInstance;
class UEquipmentInstance;

/** 通过能力的 SourceObject 访问关联装备及其来源物品。 */
UCLASS(MinimalAPI)
class UCustomGameplayAbility_FromEquipment : public UExtGameplayAbility
{
	GENERATED_BODY()

public:
	UCustomGameplayAbility_FromEquipment(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category="Ability")
	UEquipmentInstance* GetAssociatedEquipment() const;

	UFUNCTION(BlueprintCallable, Category = "Ability")
	UInventoryItemInstance* GetAssociatedItem() const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};

#undef UE_API
