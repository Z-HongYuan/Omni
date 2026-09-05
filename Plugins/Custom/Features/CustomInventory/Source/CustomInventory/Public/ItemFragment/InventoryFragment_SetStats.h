// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "InventoryItemFragmentBase.h"
#include "InventoryFragment_SetStats.generated.h"

#define UE_API CUSTOMINVENTORY_API

/**
 * 复制于CDO的动态数量
 */
UCLASS(MinimalAPI)
class UInventoryFragment_SetStats : public UInventoryItemFragmentBase
{
	GENERATED_BODY()

public:
	virtual void OnInstanceCreated(UInventoryItemInstance* Instance) const override;

	int32 GetItemStatByTag(FGameplayTag Tag) const;

protected:
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TMap<FGameplayTag, int32> InitialItemStats;
};

#undef UE_API
