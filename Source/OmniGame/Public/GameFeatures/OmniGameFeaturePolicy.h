// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFeatureStateChangeObserver.h"
#include "GameFeaturesProjectPolicies.h"

#include "OmniGameFeaturePolicy.generated.h"

#define UE_API OMNIGAME_API

class UGameFeatureData;
struct FPrimaryAssetId;

/**
 * 项目的 GameFeature 管理策略
 *
 * 职责：
 * - 通过 DefaultGame.ini 的 GameFeaturesManagerClassName 接管 GameFeaturesSubsystem 的策略
 * - InitGameFeatureManager 时挂上项目观察者（当前只有 GameplayCue 路径注册观察者）
 * - 定义 GameFeature 数据包的加载模式：DS 只加载服务器包，客户端只加载客户端包
 *
 * 注意：
 * - 需要在 DefaultGame.ini 配置才会生效（UGameFeaturesSubsystemSettings 是 config=Game）：
 *   [/Script/GameFeatures.GameFeaturesSubsystemSettings]
 *   GameFeaturesManagerClassName=/Script/OmniGame.OmniGameFeaturePolicy
 *
 * 依赖：
 * - GameplayCue 路径观察者对接 UCustomGameplayCueManager
 * - 暂未接入热更系统（需要 OnlineHotfixManager 插件），后续接入时再补热更观察者
 */
UCLASS(MinimalAPI, Config = Game)
class UOmniGameFeaturePolicy : public UDefaultGameFeaturesProjectPolicies
{
	GENERATED_BODY()

public:
	UE_API static UOmniGameFeaturePolicy& Get();

	UE_API UOmniGameFeaturePolicy(const FObjectInitializer& ObjectInitializer);

	//~UGameFeaturesProjectPolicies interface
	UE_API virtual void InitGameFeatureManager() override;
	UE_API virtual void ShutdownGameFeatureManager() override;
	UE_API virtual TArray<FPrimaryAssetId> GetPreloadAssetListForGameFeature(const UGameFeatureData* GameFeatureToLoad, bool bIncludeLoadedAssets = false) const override;
	UE_API virtual bool IsPluginAllowed(const FString& PluginURL, FString* OutReason) const override;
	UE_API virtual const TArray<FName> GetPreloadBundleStateForGameFeature() const override;
	UE_API virtual void GetGameFeatureLoadingMode(bool& bLoadClientData, bool& bLoadServerData) const override;
	//~End of UGameFeaturesProjectPolicies interface

private:
	// 策略挂到 GameFeaturesSubsystem 上的观察者集合（生命周期由子系统管理）
	UPROPERTY(Transient)
	TArray<TObjectPtr<UObject>> Observers;
};

/**
 * 观察者：GameFeature 注册时把其中 AddGameplayCuePath 动作声明的目录
 * 接入 GameplayCueManager 并重建运行时 Cue 库；注销时对称移除
 */
UCLASS()
class UOmniGameFeature_AddGameplayCuePaths : public UObject, public IGameFeatureStateChangeObserver
{
	GENERATED_BODY()

public:
	UE_API virtual void OnGameFeatureRegistering(const UGameFeatureData* GameFeatureData, const FString& PluginName, const FString& PluginURL) override;
	UE_API virtual void OnGameFeatureUnregistering(const UGameFeatureData* GameFeatureData, const FString& PluginName, const FString& PluginURL) override;
};

#undef UE_API
