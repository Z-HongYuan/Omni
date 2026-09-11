// Copyright © 2026 张鸿源. All Rights Reserved.


#include "GamPhases/CustomGamePhaseAbility.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "GamPhases/CustomGamePhaseManager.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(CustomGamePhaseAbility)

UCustomGamePhaseAbility::UCustomGamePhaseAbility(const FObjectInitializer& ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnly;
}

#if WITH_EDITOR
EDataValidationResult UCustomGamePhaseAbility::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	if (!GamePhaseTag.IsValid())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("CustomGamePhaseAbility", "GamePhaseTagNotSet", "GamePhaseTag must be set to a tag representing the current phase."));
	}

	return Result;
}
#endif

void UCustomGamePhaseAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (ActorInfo->IsNetAuthority())
	{
		UWorld* World = ActorInfo->AbilitySystemComponent->GetWorld();
		UCustomGamePhaseManager* PhaseSubsystem = UWorld::GetSubsystem<UCustomGamePhaseManager>(World);
		PhaseSubsystem->OnBeginPhase(this, Handle);
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UCustomGamePhaseAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ActorInfo->IsNetAuthority())
	{
		UWorld* World = ActorInfo->AbilitySystemComponent->GetWorld();
		UCustomGamePhaseManager* PhaseSubsystem = UWorld::GetSubsystem<UCustomGamePhaseManager>(World);
		PhaseSubsystem->OnEndPhase(this, Handle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
