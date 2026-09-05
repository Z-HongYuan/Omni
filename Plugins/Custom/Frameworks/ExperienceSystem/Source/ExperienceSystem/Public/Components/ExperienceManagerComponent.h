// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFeaturePluginOperationResult.h"
#include "LoadingScreenCheckInterface.h"
#include "Components/GameStateComponent.h"
#include "ExperienceManagerComponent.generated.h"

#define UE_API EXPERIENCESYSTEM_API

class UExperienceDefinition;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnExperienceLoaded, const UExperienceDefinition*);

enum class EExperienceLoadState : uint8
{
	// 体验未加载
	Unloaded,
	// 体验加载中
	Loading,
	// 正在加载游戏功能
	LoadingGameFeatures,
	// 正在测试延迟
	LoadingChaosTestingDelay,
	// 正在执行功能操作
	ExecutingActions,
	// 体验已加载
	Loaded,
	// 体验停用
	Deactivating
};

// Lyra中的TODO
//@TODO: Async load the experience definition itself
//@TODO: Handle failures explicitly (go into a 'completed but failed' state rather than check()-ing)
//@TODO: Do the action phases at the appropriate times instead of all at once
//@TODO: Support deactivating an experience and do the unloading actions
//@TODO: Think about what deactivation/cleanup means for preloaded assets
//@TODO: Handle deactivating game features, right now we 'leak' them enabled
// (for a client moving from experience to experience we actually want to diff the requirements and only unload some, not unload everything for them to just be immediately reloaded)
//@TODO: Handle both built-in and URL-based plugins (search for colon?)


/*
 * 体验管理器组件
 * 位于在 GS 中 , 用于管理游戏体验相关的状态和操作
 */
UCLASS(MinimalAPI)
class UExperienceManagerComponent final : public UGameStateComponent, public ILoadingScreenCheckInterface
{
	GENERATED_BODY()

public:
	UE_API UExperienceManagerComponent(const FObjectInitializer& ObjectInitializer);

	//~UActorComponent interface
	UE_API virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of UActorComponent interface

	//~ 加载屏幕检查接口
	UE_API virtual bool ShouldShowLoadingScreen(FString& OutReason) const override;
	//~ 加载屏幕检查接口

	UE_API void CallOrRegister_OnExperienceLoaded_HighPriority(FOnExperienceLoaded::FDelegate&& Delegate);
	UE_API void CallOrRegister_OnExperienceLoaded(FOnExperienceLoaded::FDelegate&& Delegate);
	UE_API void CallOrRegister_OnExperienceLoaded_LowPriority(FOnExperienceLoaded::FDelegate&& Delegate);

	// 获取当前加载的体验定义
	UE_API const UExperienceDefinition* GetCurrentExperienceChecked() const;
	// 尝试设置当前体验，无论是使用UI体验还是游戏体验
	UE_API void SetCurrentExperience(const FPrimaryAssetId& ExperienceId);
	// 体验是否已经加载完成
	UE_API bool IsExperienceLoaded() const;

protected:
	// 开始流程
	void StartExperienceLoad();
	void OnAssetLoadComplete();
	void OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result);
	void OnPluginsLoadComplete();

	//结束流程
	void CloseExperience();
	void OnAllActionsDeactivated();
	void OnActionDeactivationCompleted();

private:
	TArray<FString> GameFeaturePluginURLs;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentExperience)
	TObjectPtr<const UExperienceDefinition> CurrentExperience;

	UFUNCTION()
	void OnRep_CurrentExperience();

	FOnExperienceLoaded OnExperienceLoaded_HighPriority;
	FOnExperienceLoaded OnExperienceLoaded;
	FOnExperienceLoaded OnExperienceLoaded_LowPriority;

	int32 NumGameFeaturePluginsLoading = 0;
	int32 NumObservedPausers = 0;
	int32 NumExpectedPausers = 0;

	// 当前体验加载状态
	EExperienceLoadState CurrentLoadState = EExperienceLoadState::Unloaded;
};
#undef UE_API
