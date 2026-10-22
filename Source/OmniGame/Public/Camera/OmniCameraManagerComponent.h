// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "OmniCameraManagerComponent.generated.h"

#define UE_API OMNIGAME_API

class UGameplayCameraComponent;

/**
 * 在 DataAvailable -> DataInitialized 时，使用 PawnData.CameraAsset 接管本地玩家的相机。
 * 只接受 Pawn 上唯一且已注册的 GameplayCamera；缺少相机或配置时跳过接管，不阻塞状态推进。
 * 不创建或销毁相机；EndPlay 只停用仍由本组件控制的相机，并保留已应用的资产配置。
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
