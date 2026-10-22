// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Abilities/GameplayAbility.h"
#include "ExtGameplayAbility.generated.h"

#define UE_API ABILITYEXTENSION_API

class UExtAbilityCost;

/** 能力的自动激活策略。 */
UENUM(BlueprintType)
enum class EExtAbilityActivationPolicy : uint8
{
	/** 输入触发时尝试激活。 */
	OnInputTriggered,

	/** 输入保持期间，对尚未激活的能力持续尝试激活。 */
	WhileInputActive,

	/** 授予能力或设置新 Pawn 时，按网络执行策略尝试激活。 */
	OnSpawn
};

/** 能力与其他激活能力之间的互斥关系。 */
UENUM(BlueprintType)
enum class EExtAbilityActivationGroup : uint8
{
	/** 不参与激活组互斥。 */
	Independent,

	/** 允许被其他独占能力取消并替换。 */
	Exclusive_Replaceable,

	/** 阻止其他独占能力激活。 */
	Exclusive_Blocking,

	MAX UMETA(Hidden)
};

/** 能力激活失败时广播的蒙太奇反馈消息。 */
USTRUCT(BlueprintType)
struct FExtAbilityMontageFailureMessage
{
	GENERATED_BODY()

public:
	/** 激活失败能力所属的玩家控制器，可以为空。 */
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	/** 激活失败能力对应的 Avatar Actor。 */
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> AvatarActor = nullptr;

	/** 本次激活失败的原因标签。 */
	UPROPERTY(BlueprintReadWrite)
	FGameplayTagContainer FailureTags;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAnimMontage> FailureMontage = nullptr;
};

USTRUCT(BlueprintType)
struct FExtAbilitySimpleFailureMessage
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
 * 支持标签输入、激活策略和激活组的能力基类，提供额外消耗及失败反馈。
 * 标签关系表扩展激活条件，命中的物理材质标签会追加到效果的目标标签。
 * 默认使用本地预测和每个 Actor 一个实例；效果上下文仍由父类创建。
 */
UCLASS(MinimalAPI, Abstract, HideCategories = Input)
class UExtGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

	friend class UExtAbilitySystemComponent;

public:
	UE_API UExtGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// ~获取自定义类型的方便函数
	template <class T = UExtAbilitySystemComponent>
	T* GetExtAbilitySystemComponentFromActorInfo() const;
	UFUNCTION(BlueprintCallable, Category = "AbilityExtension|Ability", DisplayName="Get Ext ASC From Actor Info")
	UE_API UExtAbilitySystemComponent* GetExtAbilitySystemComponentFromActorInfo() const;

	template <class T = APlayerController>
	T* GetExtPlayerControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "AbilityExtension|Ability")
	UE_API AController* GetControllerFromActorInfo() const;

	template <class T = ACharacter>
	T* GetCharacterFromActorInfo() const;
	UFUNCTION(BlueprintCallable, Category = "AbilityExtension|Ability")
	UE_API ACharacter* GetCharacterFromActorInfo() const;
	// ~获取自定义类型的方便函数

	// ~控制激活组 , 将会在ASC中进行控制
	/** 检查当前已激活实例能否切换到指定激活组。 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "AbilityExtension|Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	UE_API bool CanChangeActivationGroup(EExtAbilityActivationGroup NewGroup) const;
	/** 尝试切换激活组，并同步 ASC 中的组计数和互斥关系。 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "AbilityExtension|Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	UE_API bool ChangeActivationGroup(EExtAbilityActivationGroup NewGroup);
	// ~控制激活组

	// ~控制摄像机模式
	//
	// // 设定该能力的摄像机模式。
	// UFUNCTION(BlueprintCallable, Category = "AbilityExtension|Ability")
	// UE_API void SetCameraMode(TSubclassOf<UExtCameraMode> CameraMode);
	//
	// // 清除该技能的摄像机模式。能力结束时需要时会自动调用。
	// UFUNCTION(BlueprintCallable, Category = "AbilityExtension|Ability")
	// UE_API void ClearCameraMode();
	//
	// 控制摄像机模式


	void OnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const
	{
		NativeOnAbilityFailedToActivate(FailedReason);
		K2_OnAbilityFailedToActivate(FailedReason);
	}

	EExtAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }
	EExtAbilityActivationGroup GetActivationGroup() const { return ActivationGroup; }

	void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& AbilitySpec) const;

protected:
	/** 激活失败时广播配置的文本和蒙太奇反馈。 */
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
	// UE_API virtual void GetAbilitySource(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, float& OutSourceLevel, const IExtAbilitySourceInterface*& OutAbilitySource, AActor*& OutEffectCauser) const;

	/** 当此能力被授予能力系统组件时调用。 */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityAdded")
	UE_API void K2_OnAbilityAdded();

	/** 当此能力从能力系统组件中删除时调用。 */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityRemoved")
	UE_API void K2_OnAbilityRemoved();

	/** 能力系统设置新的 Pawn Avatar 后调用。 */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnPawnAvatarSet")
	UE_API void K2_OnPawnAvatarSet();

	/** 决定输入或 Pawn 就绪时如何尝试激活。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilityExtension|Ability Activation")
	EExtAbilityActivationPolicy ActivationPolicy;

	/** 决定与其他激活能力之间的互斥关系。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilityExtension|Ability Activation")
	EExtAbilityActivationGroup ActivationGroup;

	/** 能力提交时需要检查并结算的额外消耗。 */
	UPROPERTY(EditDefaultsOnly, Instanced, Category = Costs)
	TArray<TObjectPtr<UExtAbilityCost>> AdditionalCosts;

	/** 失败标签到用户提示文本的映射，每次失败最多广播一条文本。 */
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	TMap<FGameplayTag, FText> FailureTagToUserFacingMessages;
	/** 失败标签到反馈蒙太奇的映射。 */
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> FailureTagToAnimMontage;

	/** 预留的取消日志开关，当前基类未读取此字段。 */
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	bool bLogCancelation;
};

template <class T>
T* UExtGameplayAbility::GetExtAbilitySystemComponentFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<T>(CurrentActorInfo->AbilitySystemComponent.Get()) : nullptr);
}

template <class T>
T* UExtGameplayAbility::GetExtPlayerControllerFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<T>(CurrentActorInfo->PlayerController.Get()) : nullptr);
}

template <class T>
T* UExtGameplayAbility::GetCharacterFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<T>(CurrentActorInfo->AvatarActor.Get()) : nullptr);
}

#undef UE_API
