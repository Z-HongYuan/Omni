// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Component/ExtDeathComponent.h"

#include "Data/ExtAbilitySystemTags.h"
#include "Net/UnrealNetwork.h"
#include "System/ExtAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExtDeathComponent)

UExtDeathComponent::UExtDeathComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UExtDeathComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, DeathState);
}

void UExtDeathComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UninitializeFromAbilitySystem();
	Super::EndPlay(EndPlayReason);
}

void UExtDeathComponent::OnUnregister()
{
	UninitializeFromAbilitySystem();
	Super::OnUnregister();
}

void UExtDeathComponent::InitializeWithAbilitySystem(UExtAbilitySystemComponent* InASC)
{
	if (AbilitySystemComponent == InASC) return;
	UninitializeFromAbilitySystem();
	AbilitySystemComponent = InASC;
	ApplyDeathToAbilitySystem();
}

void UExtDeathComponent::UninitializeFromAbilitySystem()
{
	// 只撤销本组件加的一份计数，不能清掉其他 Avatar 或效果的标签。
	if (AbilitySystemComponent && bAppliedDeathTags)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(ExtAbilitySystemTags::TAG_Status_Death);
		AbilitySystemComponent->RemoveLooseGameplayTag(ExtAbilitySystemTags::TAG_Gameplay_AbilityInputBlocked);
	}
	bAppliedDeathTags = false;
	AbilitySystemComponent = nullptr;
}

void UExtDeathComponent::StartDeath()
{
	if (!GetOwner()->HasAuthority() || IsDeadOrDying()) return;
	DeathState = EExtDeathState::DeathStarted;
	GetOwner()->ForceNetUpdate();
	NotifyDeathState();
}

void UExtDeathComponent::FinishDeath()
{
	if (!GetOwner()->HasAuthority() || DeathState != EExtDeathState::DeathStarted) return;
	DeathState = EExtDeathState::DeathFinished;
	GetOwner()->ForceNetUpdate();
	NotifyDeathState();
}

void UExtDeathComponent::OnRep_DeathState(EExtDeathState OldState)
{
	NotifyDeathState();
}

void UExtDeathComponent::NotifyDeathState()
{
	if (bDispatchingNotifications) return;
	TGuardValue<bool> DispatchGuard(bDispatchingNotifications, true);
	ApplyDeathToAbilitySystem();
	// 取消能力或开始回调都可能同步 FinishDeath；由最外层按顺序各发送一次。
	if (NotifiedState == EExtDeathState::NotDead && IsDeadOrDying())
	{
		NotifiedState = EExtDeathState::DeathStarted;
		OnDeathStarted.Broadcast(GetOwner());
	}
	if (NotifiedState == EExtDeathState::DeathStarted && DeathState == EExtDeathState::DeathFinished)
	{
		NotifiedState = EExtDeathState::DeathFinished;
		OnDeathFinished.Broadcast(GetOwner());
	}
}

void UExtDeathComponent::ApplyDeathToAbilitySystem()
{
	if (!AbilitySystemComponent || !IsDeadOrDying() || bAppliedDeathTags) return;
	if (AbilitySystemComponent->GetAvatarActor() != GetOwner()) return;

	bAppliedDeathTags = true;
	AbilitySystemComponent->AddLooseGameplayTag(ExtAbilitySystemTags::TAG_Status_Death);
	AbilitySystemComponent->AddLooseGameplayTag(ExtAbilitySystemTags::TAG_Gameplay_AbilityInputBlocked);
	AbilitySystemComponent->ClearAbilityInput();

	// 存活于死亡的能力由其所有者管理；普通能力结束时释放自己的任务和计时器。
	FGameplayTagContainer SurvivesDeath;
	SurvivesDeath.AddTag(ExtAbilitySystemTags::TAG_Ability_Behavior_SurvivesDeath);
	AbilitySystemComponent->CancelAbilities(nullptr, &SurvivesDeath);
}
