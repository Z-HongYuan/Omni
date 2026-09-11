// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "InteractionHelper.generated.h"

#define UE_API CUSTOMINTERACTION_API

struct FHitResult;
struct FOverlapResult;
class IInteractableTargetInterface;

/**
 * 
 */
UCLASS(MinimalAPI)
class UInteractionHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 从 Interactable(交互)接口 获取 actor
	UFUNCTION(BlueprintCallable)
	static AActor* GetActorFromInteractableTarget(TScriptInterface<IInteractableTargetInterface> InteractableTarget);

	// 从 actor 获取 Interactable(交互)接口
	UFUNCTION(BlueprintCallable)
	static void GetInteractableTargetsFromActor(AActor* Actor, TArray<TScriptInterface<IInteractableTargetInterface>>& OutInteractableTargets);

	// 从重叠结果中获取所有可交互的目标
	UE_API static void GetInteractableTargetsFromOverlapResults(const TArray<FOverlapResult>& OverlapResults, TArray<TScriptInterface<IInteractableTargetInterface>>& OutInteractableTargets);

	// 从命中结果中获取所有可交互的目标
	UE_API static void GetInteractableTargetsFromHitResult(const FHitResult& HitResult, TArray<TScriptInterface<IInteractableTargetInterface>>& OutInteractableTargets);
};

#undef UE_API
