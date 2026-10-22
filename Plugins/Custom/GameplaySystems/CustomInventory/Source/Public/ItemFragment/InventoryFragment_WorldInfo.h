// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "InventoryItemFragmentBase.h"
#include "InventoryFragment_WorldInfo.generated.h"

#define UE_API CUSTOMINVENTORY_API

/** 提供物品在世界中展示时使用的网格、名称和颜色。 */
UCLASS(MinimalAPI)
class UInventoryFragment_WorldInfo : public UInventoryItemFragmentBase
{
	GENERATED_BODY()

public:
	/** 物品的骨骼网格。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	TObjectPtr<USkeletalMesh> SkeletalMesh;

	/** 物品的显示名称。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FText DisplayName;

	/** 物品展示使用的颜色。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FLinearColor PadColor = FLinearColor::Green;;
};

#undef UE_API
