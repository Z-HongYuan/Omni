// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "UObject/Interface.h"
#include "PoolableObjectInterface.generated.h"

#define UE_API HELPERFUNCTIONS_API

UINTERFACE(MinimalAPI)
class UPoolableObjectInterface : public UInterface
{
	GENERATED_BODY()
};

/** 由对象池在复用和归还时调用的生命周期接口。 */
class IPoolableObjectInterface
{
	GENERATED_BODY()

public:
	/** 从池中取出时恢复对象状态；调用时对象已进入活跃列表。 */
	UE_API virtual void OnPoolActivated() = 0;
	/** 归还时清理本次使用产生的状态和回调，供后续复用。 */
	UE_API virtual void OnPoolDeactivated() = 0;
};

#undef UE_API
