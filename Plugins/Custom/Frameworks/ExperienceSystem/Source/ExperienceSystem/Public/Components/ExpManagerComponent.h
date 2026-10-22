// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFeaturePluginOperationResult.h"
#include "LoadingScreenCheckInterface.h"
#include "Components/GameStateComponent.h"
#include "ExpManagerComponent.generated.h"

#define UE_API EXPERIENCESYSTEM_API

class UExpDefinition;

DECLARE_MULTICAST_DELEGATE_OneParam(FExpLoaded, const UExpDefinition*);

enum class EExpLoadState : uint8
{
	/** 体验未加载。 */
	Unloaded,
	/** 正在加载体验引用的资产。 */
	Loading,
	/** 正在加载并激活 Game Feature 插件。 */
	LoadingGameFeatures,
	/** 等待可配置的加载延迟测试计时。 */
	LoadingChaosTestingDelay,
	/** 正在注册、加载并激活 Action。 */
	ExecutingActions,
	/** 体验已就绪，可以通知消费者。 */
	Loaded,
	/** 正在停用体验。 */
	Deactivating
};

// 尚未完成的加载与清理约束。
// TODO: 将体验定义本身改为异步加载。
// TODO: 显式处理加载失败，提供失败状态，避免只依赖检查。
// TODO: 按各阶段时机执行 Action，避免集中注册、加载和激活。
// TODO: 完善体验停用后的卸载流程。
// TODO: 明确预加载资产在停用和清理时的生命周期。
// TODO: 完善体验切换时的插件差异卸载；当前 CloseExperience 只请求停用插件。
// TODO: 同时支持按内置名称和 URL 指定插件。


/**
 * GameState 上的体验管理器，加载资产与插件、执行 Action，并按优先级广播就绪。
 * 服务器复制当前体验定义，客户端收到后独立加载；退出时执行停用，完整卸载仍待完善。
 */
UCLASS(MinimalAPI)
class UExpManagerComponent final : public UGameStateComponent, public ILoadingScreenCheckInterface
{
	GENERATED_BODY()

public:
	UE_API UExpManagerComponent(const FObjectInitializer& ObjectInitializer);

	//~UActorComponent interface
	UE_API virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of UActorComponent interface

	//~ 加载屏幕检查接口
	UE_API virtual bool ShouldShowLoadingScreen(FString& OutReason) const override;
	//~ 加载屏幕检查接口

	/** 已就绪时同步执行，否则在普通和低优先级通知之前调用。 */
	UE_API void CallOrRegister_OnExperienceLoaded_HighPriority(FExpLoaded::FDelegate&& Delegate);
	/** 已就绪时同步执行，否则在高优先级之后、低优先级之前调用。 */
	UE_API void CallOrRegister_OnExperienceLoaded(FExpLoaded::FDelegate&& Delegate);
	/** 已就绪时同步执行，否则等待其他优先级的就绪通知完成后调用。 */
	UE_API void CallOrRegister_OnExperienceLoaded_LowPriority(FExpLoaded::FDelegate&& Delegate);

	/** 获取已就绪的体验定义；未加载完成时触发检查。 */
	UE_API const UExpDefinition* GetCurrentExperienceChecked() const;
	/** 设置体验并启动加载；同步读取定义，要求当前没有体验，通常由服务器 GameMode 调用。 */
	UE_API void SetCurrentExperience(const FPrimaryAssetId& ExperienceId);
	/** 体验定义有效且状态为 Loaded 时返回 true。 */
	UE_API bool IsExperienceLoaded() const;

protected:
	/** 加载体验资产和插件，再执行 Action；要求当前为 Unloaded 且定义有效。 */
	void StartExperienceLoad();
	void OnAssetLoadComplete();
	void OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result);
	void OnPluginsLoadComplete();

	/** 停用插件和 Action；当前要求体验已加载，尚不支持完整的加载中退出与资产卸载。 */
	void CloseExperience();
	void OnAllActionsDeactivated();
	void OnActionDeactivationCompleted();

private:
	TArray<FString> GameFeaturePluginURLs;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentExperience)
	TObjectPtr<const UExpDefinition> CurrentExperience;

	UFUNCTION()
	void OnRep_CurrentExperience();

	FExpLoaded OnExperienceLoaded_HighPriority;
	FExpLoaded OnExperienceLoaded;
	FExpLoaded OnExperienceLoaded_LowPriority;

	int32 NumGameFeaturePluginsLoading = 0;
	int32 NumObservedPausers = 0;
	int32 NumExpectedPausers = 0;

	/** 当前体验加载阶段，用于加载界面和流程检查。 */
	EExpLoadState CurrentLoadState = EExpLoadState::Unloaded;
};
#undef UE_API
