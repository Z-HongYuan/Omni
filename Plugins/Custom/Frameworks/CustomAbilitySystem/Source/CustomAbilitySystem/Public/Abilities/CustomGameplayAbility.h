// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Abilities/GameplayAbility.h"
#include "CustomGameplayAbility.generated.h"

#define UE_API CUSTOMABILITYSYSTEM_API

class UCustomAbilityCost;

/*
 * 输入激活模式
 */
UENUM(BlueprintType)
enum class ECustomAbilityActivationPolicy : uint8
{
	// 尝试在输入触发时激活该能力。
	OnInputTriggered,

	// 在输入激活时不断尝试激活该能力。
	WhileInputActive,

	// 当分配到Pawn时尝试激活该能力。
	OnSpawn
};

/**
 *	定义了能力与其他能力的激活方式。
 */
UENUM(BlueprintType)
enum class ECustomAbilityActivationGroup : uint8
{
	// 能力独立于所有其他能力运行。
	Independent,

	// 技能被取消，并被其他独占技能所取代。
	Exclusive_Replaceable,

	// 能力阻止所有其他独占能力激活。
	Exclusive_Blocking,

	MAX UMETA(Hidden)
};

/*
 * 失败时可用于播放动画蒙太奇的失败原因
 */
USTRUCT(BlueprintType)
struct FCustomAbilityMontageFailureMessage
{
	GENERATED_BODY()

public:
	// 如果能力系统组件为玩家所有，未能激活该能力的玩家控制器
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	// 未能激活该能力的化身演员
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> AvatarActor = nullptr;

	// 这种能力失败的所有原因
	UPROPERTY(BlueprintReadWrite)
	FGameplayTagContainer FailureTags;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAnimMontage> FailureMontage = nullptr;
};

USTRUCT(BlueprintType)
struct FCustomAbilitySimpleFailureMessage
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	UPROPERTY(BlueprintReadWrite)
	FGameplayTagContainer FailureTags;

	UPROPERTY(BlueprintReadWrite)
	FText UserFacingReason;
};

/**
 * 满足自定义特性的能力基类,使用Tag进行设置
 * 1. 添加了失败处理系统,可以转发到动画或者UI
 * 2. 拥有自定义的 Getter 用于获取对应的指针,例如 ASC,Character,PlayerController
 * 3. 提供激活组控制,能够构成 独立.互斥.独占 三种类型
 * 4. 提供自定义的,额外的消耗处理
 * 5. 提供能在蓝图中使用的各种事件
 * 6. 提供激活策略,能够按下触发,持续触发,在 Give 后触发
 * 7. 提供自定义的Context上下文
 * 8. 提供命中后自动添加命中标签
 * 9. 提供激活Ability时的死亡检查
 * 10. 提供 DoesAbilitySatisfyTagRequirements 拓展 Tag 之间的关系检查
 * 11. 屏蔽原生输入,使用 Tag 进行输入处理
 */
UCLASS(MinimalAPI, Abstract, HideCategories = Input)
class UCustomGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

	friend class UCustomAbilitySystemComponent;

public:
	UE_API UCustomGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// ~获取自定义类型的方便函数
	template <class T = UCustomAbilitySystemComponent>
	T* GetCustomAbilitySystemComponentFromActorInfo() const;
	UFUNCTION(BlueprintCallable, Category = "Custom|Ability", DisplayName="Get Custom ASC From Actor Info")
	UE_API UCustomAbilitySystemComponent* GetCustomAbilitySystemComponentFromActorInfo() const;

	template <class T = APlayerController>
	T* GetCustomPlayerControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Custom|Ability")
	UE_API AController* GetControllerFromActorInfo() const;

	template <class T = ACharacter>
	T* GetCharacterFromActorInfo() const;
	UFUNCTION(BlueprintCallable, Category = "Custom|Ability")
	UE_API ACharacter* GetCharacterFromActorInfo() const;
	// ~获取自定义类型的方便函数

	// ~控制激活组 , 将会在ASC中进行控制
	// 如果请求的激活组是有效的过渡，则返回为真。
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Custom|Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	UE_API bool CanChangeActivationGroup(ECustomAbilityActivationGroup NewGroup) const;
	// 尝试更改激活组。如果成功更改，则返回为真。
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Custom|Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	UE_API bool ChangeActivationGroup(ECustomAbilityActivationGroup NewGroup);
	// ~控制激活组

	// ~控制摄像机模式
	//
	// // 设定该能力的摄像机模式。
	// UFUNCTION(BlueprintCallable, Category = "Custom|Ability")
	// UE_API void SetCameraMode(TSubclassOf<UCustomCameraMode> CameraMode);
	//
	// // 清除该技能的摄像机模式。能力结束时需要时会自动调用。
	// UFUNCTION(BlueprintCallable, Category = "Custom|Ability")
	// UE_API void ClearCameraMode();
	//
	// 控制摄像机模式


	void OnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const
	{
		NativeOnAbilityFailedToActivate(FailedReason);
		K2_OnAbilityFailedToActivate(FailedReason);
	}

	ECustomAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }
	ECustomAbilityActivationGroup GetActivationGroup() const { return ActivationGroup; }

	void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& AbilitySpec) const;

protected:
	// 能力未激活时被调用
	UE_API virtual void NativeOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;
	UFUNCTION(BlueprintImplementableEvent)
	UE_API void K2_OnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;

	//~UGameplayAbility interface
	UE_API virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	                                       FGameplayTagContainer* OptionalRelevantTags) const override;
	UE_API virtual void SetCanBeCanceled(bool bCanBeCanceled) override;
	UE_API virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	UE_API virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	// UE_API virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	// UE_API virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	UE_API virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	UE_API virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	UE_API virtual FGameplayEffectContextHandle MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const override;
	UE_API virtual void ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const override;
	UE_API virtual bool DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
	                                                      OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	//~End of UGameplayAbility interface

	UE_API virtual void OnPawnAvatarSet();

	// 在能力中添加并获取其他额外信息
	// UE_API virtual void GetAbilitySource(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, float& OutSourceLevel, const ICustomAbilitySourceInterface*& OutAbilitySource, AActor*& OutEffectCauser) const;

	/** 当此能力被授予能力系统组件时调用。 */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityAdded")
	UE_API void K2_OnAbilityAdded();

	/** 当此能力从能力系统组件中删除时调用。 */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityRemoved")
	UE_API void K2_OnAbilityRemoved();

	/** 当能力系统用Pawn化身初始化时调用。 */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnPawnAvatarSet")
	UE_API void K2_OnPawnAvatarSet();

	// 定义了该能力的激活方式。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom|Ability Activation")
	ECustomAbilityActivationPolicy ActivationPolicy;

	// 定义了该能力激活与其他能力激活之间的关系。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom|Ability Activation")
	ECustomAbilityActivationGroup ActivationGroup;

	// 激活此技能必须支付的额外消耗
	UPROPERTY(EditDefaultsOnly, Instanced, Category = Costs)
	TArray<TObjectPtr<UCustomAbilityCost>> AdditionalCosts;

	// 故障标签映射到简单错误消息
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	TMap<FGameplayTag, FText> FailureTagToUserFacingMessages;
	// 将失败标签映射到应该播放的动画蒙太奇
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> FailureTagToAnimMontage;

	// 如果为真，取消此功能时应记录额外信息。这是临时的，用于跟踪错误。
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	bool bLogCancelation;
};

template <class T>
T* UCustomGameplayAbility::GetCustomAbilitySystemComponentFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<T>(CurrentActorInfo->AbilitySystemComponent.Get()) : nullptr);
}

template <class T>
T* UCustomGameplayAbility::GetCustomPlayerControllerFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<T>(CurrentActorInfo->PlayerController.Get()) : nullptr);
}

template <class T>
T* UCustomGameplayAbility::GetCharacterFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<T>(CurrentActorInfo->AvatarActor.Get()) : nullptr);
}

#undef UE_API
