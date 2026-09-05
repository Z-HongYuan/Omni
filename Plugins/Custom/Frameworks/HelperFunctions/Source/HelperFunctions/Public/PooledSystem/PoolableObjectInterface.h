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

/**
 * 实现池化行为的接口
 */
class IPoolableObjectInterface
{
	GENERATED_BODY()

public:
	UE_API virtual void OnPoolActivated() = 0;
	UE_API virtual void OnPoolDeactivated() = 0;
};

#undef UE_API
