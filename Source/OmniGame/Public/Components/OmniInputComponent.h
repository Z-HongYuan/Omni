// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "OmniInputComponent.generated.h"

#define UE_API OMNIGAME_API

class UCustomAbilitySystemComponent;
class UCustomInputConfig;
class UExperiencePawnExtensionComponent;
class UInputComponent;
struct FInputActionValue;

/**
 * 项目的输入组件（对应 Lyra 的 ULyraHeroComponent，此处拆为输入 + 相机两个组件）
 *
 * 职责：
 * - 参与 Pawn 的初始化状态链：DataInitialized 时把 ASC 接到 PawnExtension 上（所有端都要做）
 * - 仅本地控制端：从 PawnData 取 InputConfig 绑定输入，能力输入按下/抬起转发给 ASC
 *
 * 注意：本组件必须挂在 Pawn 上，并且 PlayerController 的 InputComponentClass
 * 需要设置为 UCustomInputComponent，否则无法绑定能力输入。
 */
UCLASS(MinimalAPI)
class UOmniInputComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UE_API UOmniInputComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API virtual void OnRegister() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	//~IGameFrameworkInitStateInterface interface
	static UE_API const FName NAME_ActorFeatureName;
	static UE_API const FName NAME_BindInputsReady;
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	UE_API virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	UE_API virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	UE_API virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	UE_API virtual void CheckDefaultInitialization() override;
	//~End of IGameFrameworkInitStateInterface interface

	//~额外输入配置（由 GameFeatureAction_AddInputBinding 调用）
	// 添加额外的输入配置：只绑定能力输入（按下/抬起转发 ASC），不注册 IMC
	// （IMC 映射由 GameFeatureAction_AddInputContextMapping 负责，避免同一条 IA 被两处重复注册）
	UE_API void AddAdditionalInputConfig(const UCustomInputConfig* InputConfig);

	// 移除额外的输入配置并解绑其能力输入（Lyra 对应函数是 @TODO 空实现，这里是完整实现）
	UE_API void RemoveAdditionalInputConfig(const UCustomInputConfig* InputConfig);

	// 输入绑定是否已完成；GameFeatureAction 用它判断能否立即挂输入
	bool IsReadyToBindInputs() const { return bReadyToBindInputs; }
	//~End of 额外输入配置

protected:
	// 绑定输入，只有本地控制端才会走到这里
	// 默认实现：注册 InputConfig 的映射 + 能力输入转发给 ASC + 基础原生动作（移动/视角）
	// 项目特有的原生动作请派生类重载本函数后，用 UCustomInputComponent::BindNativeAction 自己绑定
	UE_API virtual void InitializePlayerInput(UInputComponent* PlayerInputComponent);

	// 能力输入转发到 ASC
	UE_API void Input_AbilityInputTagPressed(const FGameplayTag InputTag);
	UE_API void Input_AbilityInputTagReleased(const FGameplayTag InputTag);

	//~基础原生动作（对应 Lyra 的 Input_Move / Input_Look_Mouse）
	UE_API void Input_Move(const FInputActionValue& InputActionValue);
	UE_API void Input_Look_Mouse(const FInputActionValue& InputActionValue);

private:
	// 已绑定的输入配置，用于 EndPlay 时解绑
	UPROPERTY(Transient)
	TObjectPtr<const UCustomInputConfig> BoundInputConfig;

	// 能力输入的绑定句柄，用于 EndPlay 时解绑
	UPROPERTY(Transient)
	TArray<uint32> AbilityInputBindHandles;

	// GameFeature 注入的额外输入配置，用于查重与解绑
	UPROPERTY(Transient)
	TArray<TObjectPtr<const UCustomInputConfig>> AdditionalInputConfigs;

	// 与 AdditionalInputConfigs 按下标一一对应的能力绑定句柄组
	// （UHT 不支持 TArray 嵌套容器的反射，句柄是纯整数无需 GC 保护，故不挂 UPROPERTY）
	TArray<TArray<uint32>> AdditionalAbilityBindHandles;

	// 输入绑定完成标记（InitializePlayerInput 末尾置位）
	bool bReadyToBindInputs = false;
};

#undef UE_API
