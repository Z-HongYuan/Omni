// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "UObject/Object.h"
#include "InventoryItemFragmentBase.generated.h"

#define UE_API CUSTOMINVENTORY_API

class UInventoryItemInstance;

/**
 * 表示每个项目拥有的模块化属性
 */
UCLASS(MinimalAPI, DefaultToInstanced, EditInlineNew, Abstract)
class UInventoryItemFragmentBase : public UObject
{
	GENERATED_BODY()

public:
	virtual void OnInstanceCreated(UInventoryItemInstance* Instance) const { ; }
};

#undef UE_API
