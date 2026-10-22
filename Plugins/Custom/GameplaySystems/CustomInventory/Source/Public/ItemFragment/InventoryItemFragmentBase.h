// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "UObject/Object.h"
#include "InventoryItemFragmentBase.generated.h"

#define UE_API CUSTOMINVENTORY_API

class UInventoryItemInstance;

/** 物品定义中的模块化属性片段，可在实例创建时初始化运行时数据。 */
UCLASS(MinimalAPI, DefaultToInstanced, EditInlineNew, Abstract)
class UInventoryItemFragmentBase : public UObject
{
	GENERATED_BODY()

public:
	virtual void OnInstanceCreated(UInventoryItemInstance* Instance) const { ; }
};

#undef UE_API
