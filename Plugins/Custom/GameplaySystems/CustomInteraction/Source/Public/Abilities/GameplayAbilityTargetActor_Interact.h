// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Abilities/GameplayAbilityTargetActor_Trace.h"
#include "GameplayAbilityTargetActor_Interact.generated.h"

#define UE_API CUSTOMINTERACTION_API

/*
 * 用于获取交互对象的目标定位Actor
 */
UCLASS(MinimalAPI)
class AGameplayAbilityTargetActor_Interact : public AGameplayAbilityTargetActor_Trace
{
	GENERATED_BODY()

public:
	UE_API AGameplayAbilityTargetActor_Interact();

	UE_API virtual FHitResult PerformTrace(AActor* InSourceActor) override;
};

#undef UE_API
