// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Contra/ContraGameplayAbility_Fire.h"

#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Contra/ContraWeaponComponent.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ContraGameplayAbility_Fire)

UContraGameplayAbility_Fire::UContraGameplayAbility_Fire(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	ActivationPolicy = EExtAbilityActivationPolicy::OnInputTriggered;
}

void UContraGameplayAbility_Fire::ActivateAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                                  FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	AActor* Avatar = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	UContraWeaponComponent* Weapon = Avatar ? Avatar->FindComponentByClass<UContraWeaponComponent>() : nullptr;
	if (!Weapon || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	if (ActorInfo->IsNetAuthority())
	{
		Fire();
		if (!IsActive()) return;
		GetWorld()->GetTimerManager().SetTimer(FireTimer, this, &ThisClass::Fire, Weapon->GetFireInterval(), true);
	}
	UAbilityTask_WaitInputRelease* Release = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	Release->OnRelease.AddDynamic(this, &ThisClass::Released);
	Release->ReadyForActivation();
}

void UContraGameplayAbility_Fire::Fire()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	UContraWeaponComponent* Weapon = Avatar ? Avatar->FindComponentByClass<UContraWeaponComponent>() : nullptr;
	const APawn* Pawn = Cast<APawn>(Avatar);
	if (!Weapon || !Pawn || !Pawn->GetController())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	Weapon->TryFire();
}

void UContraGameplayAbility_Fire::Released(float TimeHeld)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UContraGameplayAbility_Fire::EndAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                             FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (!IsEndAbilityValid(Handle, ActorInfo)) return;
	if (ScopeLockCount > 0)
	{
		WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &ThisClass::EndAbility, Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled));
		return;
	}
	if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(FireTimer);
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
