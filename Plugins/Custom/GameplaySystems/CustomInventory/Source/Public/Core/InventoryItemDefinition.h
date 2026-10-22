// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Templates/SubclassOf.h"
#include "UObject/Object.h"
#include "InventoryItemDefinition.generated.h"

#define UE_API CUSTOMINVENTORY_API

class UInventoryItemFragmentBase;

/** 库存物品的静态定义，由显示名称和模块化属性片段组成。 */
UCLASS(Blueprintable, Const, Abstract, MinimalAPI)
class UInventoryItemDefinition : public UObject
{
	GENERATED_BODY()

public:
	UInventoryItemDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** 物品的显示名称。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display)
	FText DisplayName;

	/** 物品定义持有的模块化属性片段。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display, Instanced)
	TArray<TObjectPtr<UInventoryItemFragmentBase>> Fragments;

	/** 返回首个匹配类型的属性片段；类型为空或没有匹配项时返回 nullptr。 */
	const UInventoryItemFragmentBase* FindFragmentByClass(TSubclassOf<UInventoryItemFragmentBase> FragmentClass) const;
};

#undef UE_API
