// Copyright © 2026 张鸿源. All Rights Reserved.


#include "AbilitySystem/Abilities/OmniGameplayAbility_Jump.h"

#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "GameFramework/Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniGameplayAbility_Jump)

UOmniGameplayAbility_Jump::UOmniGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	ActivationPolicy = EExtAbilityActivationPolicy::OnInputTriggered;
}

bool UOmniGameplayAbility_Jump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
                                                   FGameplayTagContainer* OptionalRelevantTags) const
{
	// 先检查角色是否能够跳跃。
	const ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr;

	bool bCanActivate = Character && Character->CanJump();

	// 再检查能力的标签、消耗等通用激活条件。
	bCanActivate = bCanActivate && Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);

	return bCanActivate;
}

void UOmniGameplayAbility_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	ACharacter* Character = GetCharacterFromActorInfo();
	if (!Character || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 与 Lyra 一致：由本地控制端驱动跳跃，移动同步交给 CharacterMovement。
	if (Character->IsLocallyControlled() && !Character->bPressedJump)
	{
		Character->UnCrouch();
		Character->Jump();
	}

	// 同时检查已经松开的输入，避免激活与松开发生在同一帧时遗漏。
	UAbilityTask_WaitInputRelease* ReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	ReleaseTask->OnRelease.AddDynamic(this, &ThisClass::OnInputReleased);
	ReleaseTask->ReadyForActivation();
}

void UOmniGameplayAbility_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (!IsEndAbilityValid(Handle, ActorInfo)) return;

	// 与父类保持一致：作用域锁释放后再执行结束及清理。
	if (ScopeLockCount > 0)
	{
		WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &ThisClass::EndAbility, Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled));
		return;
	}

	if (ACharacter* Character = GetCharacterFromActorInfo())
	{
		if (Character->IsLocallyControlled() && Character->bPressedJump)
		{
			Character->StopJumping();
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UOmniGameplayAbility_Jump::OnInputReleased(float TimeHeld)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
