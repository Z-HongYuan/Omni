// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Abilities/GameplayAbilityTargetActor_Trace.h"
#include "GameplayAbilityTargetActor_Interact.generated.h"

#define UE_API CUSTOMINTERACTION_API

/** 通过线追踪获取交互命中结果，并更新世界准星位置。 */
UCLASS(MinimalAPI)
class AGameplayAbilityTargetActor_Interact : public AGameplayAbilityTargetActor_Trace
{
	GENERATED_BODY()

public:
	UE_API AGameplayAbilityTargetActor_Interact();

	UE_API virtual FHitResult PerformTrace(AActor* InSourceActor) override;
};

#undef UE_API
