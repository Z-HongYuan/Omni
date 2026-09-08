// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFeatures/GameFeatureAction_WorldActionBase.h"
#include "UObject/ObjectKey.h"

#include "GameFeatureAction_SplitscreenConfig.generated.h"

#define UE_API OMNIGAME_API

class UObject;
struct FGameFeatureDeactivatingContext;
struct FGameFeatureStateChangeContext;
struct FWorldContext;

/**
 * 配置分屏开关的 GameFeatureAction
 *
 * 职责：
 * - 功能激活时对每个命中的 WorldContext 投一张"禁用分屏"票（计数引用），
 *   第 1 张票生效时强制关闭 GameViewport 的分屏
 * - 功能反激活时按票数逐一退票，票数归零才恢复分屏，保证多功能叠加时正确复原
 */
UCLASS(MinimalAPI, meta = (DisplayName = "分屏配置"))
class UGameFeatureAction_SplitscreenConfig final : public UGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

public:
	//~UGameFeatureAction interface
	UE_API virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	//~End of UGameFeatureAction interface

	UPROPERTY(EditAnywhere, Category=Action)
	bool bDisableSplitscreen = true;

private:
	//~UGameFeatureAction_WorldActionBase interface
	UE_API virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;
	//~End of UGameFeatureAction_WorldActionBase interface

	// 本功能对各视口投出的票（反激活时按此退票）
	TArray<FObjectKey> LocalDisableVotes;

	// 全局票箱：key=视口，value=当前累计票数（多个功能可同时对同一视口禁用分屏）
	static TMap<FObjectKey, int32> GlobalDisableVotes;
};

#undef UE_API
