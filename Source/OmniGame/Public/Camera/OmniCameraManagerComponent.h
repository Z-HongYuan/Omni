// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "OmniCameraManagerComponent.generated.h"

#define UE_API OMNIGAME_API

class UGameplayCameraComponent;

/**
 * 摄像机管理器：消费 PawnData.CameraAsset，控制 Pawn 上已有且已注册的唯一 GameplayCamera，不创建或销毁摄像机。
 * 沿用 Lyra HeroComponent 的注册/注销和状态链，在 DataAvailable -> DataInitialized 时接管本地相机。
 * 缺少相机或配置不阻塞初始化；蓝图/GF 动态增删相机后调用 RefreshCamera，控制切换和 PawnRestart 自动刷新。
 * EndPlay 仅停用仍由本管理器控制的相机；应用过的资产配置保留，不参与 ASC 绑定或输入处理。
 * 与完整 HeroComponent 的差异：使用 GameplayCameras 资产；尚未迁入面向所有 Pawn 的观战支持和能力相机模式覆盖。
 */
UCLASS(MinimalAPI, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UOmniCameraManagerComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UE_API UOmniCameraManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API virtual void OnRegister() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	static UE_API const FName NAME_ActorFeatureName;
	UE_API virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	UE_API virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	UE_API virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	UE_API virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	UE_API virtual void CheckDefaultInitialization() override;

	UFUNCTION(BlueprintPure, Category = "Omni|Camera")
	UGameplayCameraComponent* GetCameraComponent() const { return CameraComponent.Get(); }

private:
	void UpdateCamera();
	void ReleaseCamera();
	void ReportCameraIssue(FName Issue, const TCHAR* Message);

	UPROPERTY(Transient)
	TWeakObjectPtr<UGameplayCameraComponent> CameraComponent;

	TWeakObjectPtr<APlayerController> CameraController;
	FName LastCameraIssue;
};

#undef UE_API
