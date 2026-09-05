// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "InventoryItemFragmentBase.h"
#include "InventoryFragment_WorldInfo.generated.h"

#define UE_API CUSTOMINVENTORY_API

/**
 * 用于在世界中展示的Mesh外观
 */
UCLASS(MinimalAPI)
class UInventoryFragment_WorldInfo : public UInventoryItemFragmentBase
{
	GENERATED_BODY()

public:
	// 库存物品代表的模型
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	TObjectPtr<USkeletalMesh> SkeletalMesh;

	// 库存物品的名称
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FText DisplayName;

	// 库存物品的材质,颜色
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FLinearColor PadColor = FLinearColor::Green;;
};

#undef UE_API
