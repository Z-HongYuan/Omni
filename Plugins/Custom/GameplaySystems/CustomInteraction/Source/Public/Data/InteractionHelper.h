// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "InteractionHelper.generated.h"

#define UE_API CUSTOMINTERACTION_API

struct FHitResult;
struct FOverlapResult;
class IInteractableTargetInterface;

/** 从 Actor、组件和碰撞结果中收集交互接口。 */
UCLASS(MinimalAPI)
class UInteractionHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** 返回交互目标 Actor；组件目标返回其 Owner，其他对象类型尚不支持。 */
	UFUNCTION(BlueprintCallable)
	static AActor* GetActorFromInteractableTarget(TScriptInterface<IInteractableTargetInterface> InteractableTarget);

	/** 将 Actor 自身和其组件实现的交互接口追加到输出数组。 */
	UFUNCTION(BlueprintCallable)
	static void GetInteractableTargetsFromActor(AActor* Actor, TArray<TScriptInterface<IInteractableTargetInterface>>& OutInteractableTargets);

	/** 将重叠结果中 Actor 和组件的交互接口去重后追加到输出数组。 */
	UE_API static void GetInteractableTargetsFromOverlapResults(const TArray<FOverlapResult>& OverlapResults, TArray<TScriptInterface<IInteractableTargetInterface>>& OutInteractableTargets);

	/** 将命中 Actor 和组件的交互接口去重后追加到输出数组。 */
	UE_API static void GetInteractableTargetsFromHitResult(const FHitResult& HitResult, TArray<TScriptInterface<IInteractableTargetInterface>>& OutInteractableTargets);
};

#undef UE_API
