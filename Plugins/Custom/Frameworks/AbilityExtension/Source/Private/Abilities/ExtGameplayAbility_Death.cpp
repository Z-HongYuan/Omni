// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Abilities/ExtGameplayAbility_Death.h"

#include "Component/ExtHealthComponent.h"
#include "Data/ExtAbilitySystemTags.h"
#include "LogAbilityExtension.h"
#include "System/ExtAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExtGameplayAbility_Death)

UExtGameplayAbility_Death::UExtGameplayAbility_Death(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	// CDO 中配置触发器
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = ExtAbilitySystemTags::TAG_GameplayEvent_Death;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UExtGameplayAbility_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	check(ActorInfo);
	UExtAbilitySystemComponent* ExtASC = CastChecked<UExtAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());

	// 先取消普通技能；保留标有 AvoidDeathClear 的技能和当前死亡技能。
	FGameplayTagContainer AbilityTypesToIgnore;
	AbilityTypesToIgnore.AddTag(ExtAbilitySystemTags::TAG_Ability_Behavior_AvoidDeathClear);
	ExtASC->CancelAbilities(nullptr, &AbilityTypesToIgnore, this);

	SetCanBeCanceled(false);

	// 独占阻塞组只阻止其他独占技能，独立技能的死亡限制由标签配置负责。
	if (!ChangeActivationGroup(EExtAbilityActivationGroup::Exclusive_Blocking))
	{
		UE_LOG(LogAbilityExtension, Error, TEXT("Death ability [%s] failed to change activation group to blocking."), *GetName());
	}

	if (bAutoStartDeath)
	{
		StartDeath();
	}

	// 最后进入蓝图事件，由具体玩法安排表现和结束时机。
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UExtGameplayAbility_Death::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (!IsEndAbilityValid(Handle, ActorInfo)) return;

	if (ScopeLockCount > 0)
	{
		WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &ThisClass::EndAbility, Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled));
		return;
	}

	// 蓝图即使没有显式收尾，也会在结束技能时完成已开始的死亡流程。
	FinishDeath();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UExtGameplayAbility_Death::StartDeath()
{
	if (UExtHealthComponent* HealthComponent = UExtHealthComponent::FindHealthComponent(GetAvatarActorFromActorInfo()))
	{
		HealthComponent->StartDeath();
	}
}

void UExtGameplayAbility_Death::FinishDeath()
{
	if (UExtHealthComponent* HealthComponent = UExtHealthComponent::FindHealthComponent(GetAvatarActorFromActorInfo()))
	{
		HealthComponent->FinishDeath();
	}
}
