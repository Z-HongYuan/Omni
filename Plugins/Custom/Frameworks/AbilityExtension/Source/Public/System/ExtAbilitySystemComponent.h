// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "Abilities/ExtGameplayAbility.h"
#include "ExtAbilitySystemComponent.generated.h"

#define UE_API ABILITYEXTENSION_API

class UExtAbilityTagRelationshipMapping;
enum class EExtAbilityActivationGroup : uint8;
class UExtGameplayAbility;

/**
 * 按帧处理标签输入，维护能力激活组并通过标签关系表扩展激活和取消条件。
 * Pawn Avatar 就绪时初始化能力、全局效果和动画标签绑定，再尝试 OnSpawn 激活。
 */
UCLASS(MinimalAPI, meta=(BlueprintSpawnableComponent))
class UExtAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UE_API UExtAbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent interface
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of UActorComponent interface

	/** 初始化 Owner 与 Avatar；仅新 Pawn Avatar 触发能力、全局效果和动画绑定通知。 */
	UE_API virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

	// 按条件取消能力。
	typedef TFunctionRef<bool(const UExtGameplayAbility* ExtAbility, FGameplayAbilitySpecHandle Handle)> TShouldCancelAbilityFunc;
	/** 取消满足条件且允许取消的活跃能力实例，可选择复制取消通知。 */
	UE_API void CancelAbilitiesByFunc(const TShouldCancelAbilityFunc& ShouldCancelFunc, bool bReplicateCancelAbility);
	UE_API void CancelInputActivatedAbilities(bool bReplicateCancelAbility);

	/** 按精确输入标签记录本帧按下事件，并保持按住状态。 */
	UE_API void AbilityInputTagPressed(const FGameplayTag& InputTag);
	/** 记录本帧释放事件，并移除相应的按住状态。 */
	UE_API void AbilityInputTagReleased(const FGameplayTag& InputTag);

	/** 由外部逐帧调用，批量处理按住、按下和释放；输入阻塞时清空缓存。 */
	UE_API void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	UE_API void ClearAbilityInput();

	// 激活组维护。
	UE_API bool IsActivationGroupBlocked(EExtAbilityActivationGroup Group) const;
	UE_API void AddAbilityToActivationGroup(EExtAbilityActivationGroup Group, UExtGameplayAbility* ExtAbility);
	UE_API void RemoveAbilityFromActivationGroup(EExtAbilityActivationGroup Group, UExtGameplayAbility* ExtAbility);
	UE_API void CancelActivationGroupAbilities(EExtAbilityActivationGroup Group, UExtGameplayAbility* IgnoreExtAbility, bool bReplicateCancelAbility);

	/** 使用设置中的 GameplayEffect 为自身添加动态授予标签。 */
	UE_API void AddDynamicTagGameplayEffect(const FGameplayTag& Tag);
	/** 移除使用通用动态标签效果类且授予指定标签的活跃效果。 */
	UE_API void RemoveDynamicTagGameplayEffect(const FGameplayTag& Tag);

	/** 查询能力句柄和激活预测键对应的目标数据；未找到时不修改输出。 */
	UE_API void GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, FGameplayAbilityActivationInfo ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle);

	/** 设置标签关系表，传入空指针时清除映射。 */
	UE_API void SetTagRelationshipMapping(UExtAbilityTagRelationshipMapping* NewMapping);

	/** 将关系表中匹配的必需标签和阻塞标签追加到输出容器。 */
	UE_API void GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const;

	UE_API void TryActivateAbilitiesOnSpawn();

public:
	UE_API virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	UE_API virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;

	UE_API virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;
	UE_API virtual void NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason) override;
	UE_API virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;
	UE_API virtual void ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags,
	                                                   const FGameplayTagContainer& CancelTags) override;
	UE_API virtual void HandleChangeAbilityCanBeCanceled(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bCanBeCanceled) override;

protected:
	/** 以不可靠客户端 RPC 通知能力激活失败，供本地反馈使用。 */
	UFUNCTION(Client, Unreliable)
	UE_API void ClientNotifyAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);

	UE_API void HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);

private:
	/** 扩展能力激活、阻塞和取消条件的标签关系表。 */
	UPROPERTY()
	TObjectPtr<UExtAbilityTagRelationshipMapping> TagRelationshipMapping;

	/** 本帧收到按下输入的能力句柄。 */
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	/** 本帧收到释放输入的能力句柄。 */
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	/** 持续按住的能力句柄，直到释放或清空输入。 */
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;

	/** 各激活组当前运行的能力数量。 */
	int32 ActivationGroupCounts[static_cast<uint8>(EExtAbilityActivationGroup::MAX)];
};

#undef UE_API
