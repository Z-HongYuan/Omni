// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "OmniCameraManagerComponent.generated.h"

#define UE_API OMNIGAME_API

class UCameraAsset;
class UExperiencePawnExtensionComponent;
class UGameplayCameraComponent;

/**
 * 插件并不会添加相机组件,需要在附加的组件中有相机才能进行管理
 *
 * 本组件只负责"管理"：用 Find（FindComponentByClass）从 Pawn 附加的组件中查找引擎相机
 * UGameplayCameraComponent（Pawn 自带或 GameFeatureAction_AddComponents 平级添加均可），
 * 设置 PawnData 的相机资产并在本地控制端激活；找不到时仅告警并跳过相机相关流程，不会自己创建相机。
 */
UCLASS(MinimalAPI)
class UOmniCameraManagerComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UE_API UOmniCameraManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API virtual void OnRegister() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	//~IGameFrameworkInitStateInterface interface
	static UE_API const FName NAME_ActorFeatureName;
	static UE_API const FName NAME_CameraReady;
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	UE_API virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	UE_API virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	UE_API virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	UE_API virtual void CheckDefaultInitialization() override;
	//~End of IGameFrameworkInitStateInterface interface

	// 从当前 PawnData 读取相机资产（可能为空）
	UFUNCTION(BlueprintPure, Category = "Omni|Camera")
	UE_API UCameraAsset* GetCameraAssetFromPawnData() const;

protected:
	// 相机就绪钩子（本地控制端才会调用）：C++ 派生类重载本函数，蓝图派生类重载 K2_OnCameraReady（默认转发过去）
	UE_API virtual void OnCameraReady(UCameraAsset* CameraAsset);

	UFUNCTION(BlueprintImplementableEvent, Category = "Omni|Camera", meta = (DisplayName = "OnCameraReady"))
	UE_API void K2_OnCameraReady(UCameraAsset* CameraAsset);

private:
	// 查找 Pawn 附加组件里的引擎相机组件并缓存（本组件不创建相机，找不到时返回空）
	UE_API UGameplayCameraComponent* FindGameplayCameraComponent();

	// 从 Pawn 附加组件中查找到的引擎相机组件（运行时 Find 填充缓存）
	UPROPERTY(Transient)
	TObjectPtr<UGameplayCameraComponent> GameplayCameraComponent;
};

#undef UE_API
