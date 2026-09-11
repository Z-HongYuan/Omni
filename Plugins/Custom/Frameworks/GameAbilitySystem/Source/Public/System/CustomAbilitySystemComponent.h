// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "Abilities/CustomGameplayAbility.h"
#include "CustomAbilitySystemComponent.generated.h"

#define UE_API GAMEABILITYSYSTEM_API

class UCustomAbilityTagRelationshipMapping;
enum class ECustomAbilityActivationGroup : uint8;
class UCustomGameplayAbility;

/*
 * 1. 基于Tag触发技能的 帧批处理输入系统（Input Buffering）
 * 2. 技能激活组（Activation Groups）
 * 3. 标签关系映射（Tag Relationship Mapping）
 * 4. 条件批量取消技能
 * 5. 动态标签 GameplayEffect
 * 6. Spawn 时自动激活
 */
UCLASS(MinimalAPI, meta=(BlueprintSpawnableComponent))
class UCustomAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UE_API UCustomAbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent interface
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of UActorComponent interface

	// 初始化ASC信息
	UE_API virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

	// 取消技能
	typedef TFunctionRef<bool(const UCustomGameplayAbility* CustomAbility, FGameplayAbilitySpecHandle Handle)> TShouldCancelAbilityFunc;
	UE_API void CancelAbilitiesByFunc(const TShouldCancelAbilityFunc& ShouldCancelFunc, bool bReplicateCancelAbility);
	UE_API void CancelInputActivatedAbilities(bool bReplicateCancelAbility);

	// 技能输入和释放,向帧处理器添加需要处理的技能
	UE_API void AbilityInputTagPressed(const FGameplayTag& InputTag);
	UE_API void AbilityInputTagReleased(const FGameplayTag& InputTag);

	// 处理技能输入,帧处理器
	UE_API void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	UE_API void ClearAbilityInput();

	// 技能激活组
	UE_API bool IsActivationGroupBlocked(ECustomAbilityActivationGroup Group) const;
	UE_API void AddAbilityToActivationGroup(ECustomAbilityActivationGroup Group, UCustomGameplayAbility* CustomAbility);
	UE_API void RemoveAbilityFromActivationGroup(ECustomAbilityActivationGroup Group, UCustomGameplayAbility* CustomAbility);
	UE_API void CancelActivationGroupAbilities(ECustomAbilityActivationGroup Group, UCustomGameplayAbility* IgnoreCustomAbility, bool bReplicateCancelAbility);

	// 使用游戏效果添加指定的动态赋予标签。
	UE_API void AddDynamicTagGameplayEffect(const FGameplayTag& Tag);
	// 移除所有用于添加指定动态赋予标签的游戏效果的活跃实例。
	UE_API void RemoveDynamicTagGameplayEffect(const FGameplayTag& Tag);

	// 获取与给定能力句柄和激活信息关联的能力目标数据
	UE_API void GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, FGameplayAbilityActivationInfo ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle);

	/** 设置当前标记关系映射，如果为null，则将其清除 */
	UE_API void SetTagRelationshipMapping(UCustomAbilityTagRelationshipMapping* NewMapping);

	/** 查看能力标签并收集其他必需和阻止标签 */
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
	/** 通知客户某个能力未能激活 */
	UFUNCTION(Client, Unreliable)
	UE_API void ClientNotifyAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);

	UE_API void HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);

private:
	// 如果设置了此表，则用于查找激活和取消的标签关系
	UPROPERTY()
	TObjectPtr<UCustomAbilityTagRelationshipMapping> TagRelationshipMapping;

	// 每帧需要触发的技能
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	// 每帧需要释放的技能
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	// 每帧需要持续触发的技能
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;

	// 每个激活组中运行的技能数量。
	int32 ActivationGroupCounts[static_cast<uint8>(ECustomAbilityActivationGroup::MAX)];
};

#undef UE_API
