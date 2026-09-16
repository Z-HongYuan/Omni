// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "OmniInputManagerComponent.generated.h"

#define UE_API OMNIGAME_API

class UExtInputComponent;
class UExtInputConfig;
struct FInputActionValue;

/**
 * 输入管理器：消费 PawnData.InputConfig，绑定移动、鼠标视角，并将能力输入 Tag 转交 PawnExtension 当前的 ASC。
 * 沿用 Lyra HeroComponent 的注册/注销和状态链，在 DataAvailable -> DataInitialized 时绑定本地输入。
 * PlayerState 配对与 ASC 绑定由 PawnInitialization 负责；空输入配置不阻塞，EndPlay 只移除自身绑定。
 * 完成绑定后向 Pawn 和 PC 发送 BindInputsNow 扩展事件；IMC 由项目 GF 动作管理。
 * 与完整 HeroComponent 的差异：手柄视角和用户改键界面尚未迁入。
 */
UCLASS(MinimalAPI, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UOmniInputManagerComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UE_API UOmniInputManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API virtual void OnRegister() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	static UE_API const FName NAME_ActorFeatureName;
	static UE_API const FName NAME_BindInputsNow;
	UE_API virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	UE_API virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	UE_API virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	UE_API virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	UE_API virtual void CheckDefaultInitialization() override;

protected:
	// 派生玩法替换原生动作，ASC 输入转发及句柄清理仍沿用同一条生命周期。
	UE_API virtual void BindNativeInputActions(UExtInputComponent* Input, const UExtInputConfig* Config, TArray<uint32>& Handles);

private:
	void RefreshInputBind();
	void ReleaseInputBind();

	void Input_Move(const FInputActionValue& Value);
	void Input_LookMouse(const FInputActionValue& Value);

	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);

	TWeakObjectPtr<UExtInputComponent> BoundInputComponent;
	TWeakObjectPtr<APlayerController> BoundController;

	UPROPERTY(Transient)
	TObjectPtr<const UExtInputConfig> BoundInputConfig;

	TArray<uint32> BindingHandles;
};

#undef UE_API
