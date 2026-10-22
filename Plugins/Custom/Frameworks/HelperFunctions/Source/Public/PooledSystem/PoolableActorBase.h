// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "PoolableObjectInterface.h"
#include "System/ExtTaggedActor.h"
#include "PoolableActorBase.generated.h"

#define UE_API HELPERFUNCTIONS_API

/** 为池化 Actor 提供激活、休眠及蓝图回调。 */
UCLASS(MinimalAPI)
class APoolableActorBase : public AExtTaggedActor, public IPoolableObjectInterface
{
	GENERATED_BODY()

public:
	UE_API APoolableActorBase();

	// IPoolableObjectInterface
	UE_API virtual void OnPoolActivated() override;
	UFUNCTION(BlueprintImplementableEvent)
	UE_API void K2_OnPoolActivated();

	UE_API virtual void OnPoolDeactivated() override;
	UFUNCTION(BlueprintImplementableEvent)
	UE_API void K2_OnPoolDeactivated();
	// End IPoolableObjectInterface
};

#undef UE_API
