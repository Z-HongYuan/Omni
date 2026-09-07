// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "OmniCameraComponent.generated.h"

#define UE_API OMNIGAME_API

class UCameraAsset;
class UExperiencePawnExtensionComponent;
class UGameplayCameraComponent;

/**
 * 项目的相机组件（对应 Lyra 的 HeroComponent 中相机部分 + ULyraCameraComponent）
 *
 * 职责：
 * - 参与 Pawn 的初始化状态链，内部持有一个引擎的 UGameplayCameraComponent
 * - DataInitialized 时从 PawnData 取相机资产，设置到 UGameplayCameraComponent::CameraReference
 * - GameplayReady 且本地控制端时，激活相机系统
 *
 * 注意：相机只在本地控制端有意义，Bot 上本组件会正常推进状态链但不会激活相机。
 * 相机的状态切换（第三人称/瞄准/倒地等）请在 UCameraAsset 内部用 CameraDirector 配置，
 * 不要在这里做多相机资产的切换。
 */
UCLASS(MinimalAPI)
class UOmniCameraComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UE_API UOmniCameraComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

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

	// 获取内部的引擎相机组件
	UFUNCTION(BlueprintPure, Category = "Omni|Camera")
	UGameplayCameraComponent* GetGameplayCameraComponent() const { return GameplayCameraComponent; }

	// 从当前 PawnData 读取相机资产（可能为空）
	UFUNCTION(BlueprintPure, Category = "Omni|Camera")
	UE_API UCameraAsset* GetCameraAssetFromPawnData() const;

protected:
	// 相机就绪钩子（本地控制端才会调用），派生类可重载
	UE_API virtual void OnCameraReady(UCameraAsset* CameraAsset);

	UFUNCTION(BlueprintImplementableEvent, Category = "Omni|Camera", meta = (DisplayName = "OnCameraReady"))
	UE_API void K2_OnCameraReady(UCameraAsset* CameraAsset);

private:
	// 引擎的相机组件，负责实际的相机运行
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Omni|Camera", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGameplayCameraComponent> GameplayCameraComponent;
};

#undef UE_API
