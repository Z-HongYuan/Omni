// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFeatureAction.h"
#include "GameFeaturesSubsystem.h"

#include "GameFeatureAction_WorldActionBase.generated.h"

#define UE_API OMNIGAME_API

class FDelegateHandle;
class UGameInstance;
class UObject;
struct FGameFeatureActivatingContext;
struct FGameFeatureDeactivatingContext;
struct FGameFeatureStateChangeContext;
struct FWorldContext;

/**
 * 希望针对特定 World 做事情的所有 GameFeatureAction 的基类
 *
 * 激活时:
 * 1. 监听 OnStartGameInstance（应对"功能先激活、世界后创建"的时序，如 DS 的地图加载）
 * 2. 对所有已存在且匹配上下文的 WorldContext 立即执行 AddToWorld
 */
UCLASS(Abstract)
class UGameFeatureAction_WorldActionBase : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	//~ Begin UGameFeatureAction interface
	UE_API virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	UE_API virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	//~ End UGameFeatureAction interface

private:
	UE_API void HandleGameInstanceStart(UGameInstance* GameInstance, FGameFeatureStateChangeContext ChangeContext);

	/** 子类覆写：把该功能"加入世界"的具体逻辑 */
	UE_API virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) PURE_VIRTUAL(UGameFeatureAction_WorldActionBase::AddToWorld,);

	// 每个 GameFeatureStateChangeContext 对应一个 GameInstance 启动委托句柄
	TMap<FGameFeatureStateChangeContext, FDelegateHandle> GameInstanceStartHandles;
};

#undef UE_API
