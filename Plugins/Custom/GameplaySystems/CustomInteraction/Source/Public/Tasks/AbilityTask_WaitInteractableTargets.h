// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Abilities/Tasks/AbilityTask.h"
#include "Engine/CollisionProfile.h"
#include "AbilityTask_WaitInteractableTargets.generated.h"

#define UE_API CUSTOMINTERACTION_API

class IInteractableTargetInterface;
struct FInteractionOption;
struct FInteractionQuery;

/** 可用交互选项变化时触发的委托。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractableObjectsChangedEvent, const TArray<FInteractionOption>&, InteractableOptions);

/** 抽象交互任务基类，提供瞄准、追踪和可用选项筛选逻辑。 */
UCLASS(MinimalAPI, Abstract)
class UAbilityTask_WaitInteractableTargets : public UAbilityTask
{
	GENERATED_BODY()

public:
	/** 可用交互选项列表变化时广播。 */
	UPROPERTY(BlueprintAssignable)
	FInteractableObjectsChangedEvent InteractableObjectsChanged;

protected:
	static void LineTrace(FHitResult& OutHitResult, const UWorld* World, const FVector& Start, const FVector& End, FName ProfileName, const FCollisionQueryParams Params);

	void AimWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart, float MaxRange, FVector& OutTraceEnd, bool bIgnorePitch = false) const;

	static bool ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection, FVector AbilityCenter, float AbilityRange, FVector& ClippedPosition);

	void UpdateInteractableOptions(const FInteractionQuery& InteractQuery, const TArray<TScriptInterface<IInteractableTargetInterface>>& InteractableTargets);

	FCollisionProfileName TraceProfile;

	/** 是否允许碰撞命中结果改变瞄准俯仰角。 */
	bool bTraceAffectsAimPitch = true;

	TArray<FInteractionOption> CurrentOptions;
};

#undef UE_API
