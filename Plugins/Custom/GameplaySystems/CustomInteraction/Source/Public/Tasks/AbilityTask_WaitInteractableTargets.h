// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Abilities/Tasks/AbilityTask.h"
#include "Engine/CollisionProfile.h"
#include "AbilityTask_WaitInteractableTargets.generated.h"

#define UE_API CUSTOMINTERACTION_API

class IInteractableTargetInterface;
struct FInteractionOption;
struct FInteractionQuery;

// 交互对象改变委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractableObjectsChangedEvent, const TArray<FInteractionOption>&, InteractableOptions);

/**
 * 异步等待可交互对象列表
 * 异步获取可交互对象列表
 * 蓝图中无法使用,只是抽离通用函数用作父类
 */
UCLASS(MinimalAPI, Abstract)
class UAbilityTask_WaitInteractableTargets : public UAbilityTask
{
	GENERATED_BODY()

public:
	// 可交互对象变更
	UPROPERTY(BlueprintAssignable)
	FInteractableObjectsChangedEvent InteractableObjectsChanged;

protected:
	static void LineTrace(FHitResult& OutHitResult, const UWorld* World, const FVector& Start, const FVector& End, FName ProfileName, const FCollisionQueryParams Params);

	void AimWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart, float MaxRange, FVector& OutTraceEnd, bool bIgnorePitch = false) const;

	static bool ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection, FVector AbilityCenter, float AbilityRange, FVector& ClippedPosition);

	void UpdateInteractableOptions(const FInteractionQuery& InteractQuery, const TArray<TScriptInterface<IInteractableTargetInterface>>& InteractableTargets);

	FCollisionProfileName TraceProfile;

	// 轨迹会影响瞄准的角度吗？
	bool bTraceAffectsAimPitch = true;

	TArray<FInteractionOption> CurrentOptions;
};

#undef UE_API
