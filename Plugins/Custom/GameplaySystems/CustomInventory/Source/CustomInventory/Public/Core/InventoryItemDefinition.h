// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Templates/SubclassOf.h"
#include "UObject/Object.h"
#include "InventoryItemDefinition.generated.h"

#define UE_API CUSTOMINVENTORY_API

class UInventoryItemFragmentBase;

/**
 * 库存项目的定义
 */
UCLASS(Blueprintable, Const, Abstract, MinimalAPI)
class UInventoryItemDefinition : public UObject
{
	GENERATED_BODY()

public:
	UInventoryItemDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 物品的命名
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display)
	FText DisplayName;

	// 模块化的属性片段
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display, Instanced)
	TArray<TObjectPtr<UInventoryItemFragmentBase>> Fragments;

	// 没有属性片段的时候，返回null
	const UInventoryItemFragmentBase* FindFragmentByClass(TSubclassOf<UInventoryItemFragmentBase> FragmentClass) const;
};

#undef UE_API
