// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "InventoryItemFragmentBase.h"
#include "Styling/SlateBrush.h"
#include "InventoryFragment_UIInfo.generated.h"

#define UE_API CUSTOMINVENTORY_API

/**
 * 提供给UI使用的物品属性
 */
UCLASS(MinimalAPI)
class UInventoryFragment_UIInfo : public UInventoryItemFragmentBase
{
	GENERATED_BODY()

public:
	// 图标
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FSlateBrush Brush;

	// // 数量图标
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	// FSlateBrush AmmoBrush;

	// 显示名称
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FText DisplayName;
};

#undef UE_API
