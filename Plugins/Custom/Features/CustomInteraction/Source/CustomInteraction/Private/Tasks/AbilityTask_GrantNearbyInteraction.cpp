// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Tasks/AbilityTask_GrantNearbyInteraction.h"
#include "Engine/OverlapResult.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"
#include "Core/InteractableTargetInterface.h"
#include "Core/InteractionOption.h"
#include "Data/InteractionHelper.h"
#include "Data/InteractionQuery.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilityTask_GrantNearbyInteraction)

void UAbilityTask_GrantNearbyInteraction::OnDestroy(bool AbilityEnded)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(QueryTimerHandle);
	}

	Super::OnDestroy(AbilityEnded);
}

void UAbilityTask_GrantNearbyInteraction::Activate()
{
	SetWaitingOnAvatar();

	UWorld* World = GetWorld();
	World->GetTimerManager().SetTimer(QueryTimerHandle, this, &ThisClass::QueryInteractables, InteractionScanRate, true);
}

UAbilityTask_GrantNearbyInteraction* UAbilityTask_GrantNearbyInteraction::GrantAbilitiesForNearbyInteractors(UGameplayAbility* OwningAbility, ECollisionChannel TraceChannel, float InteractionScanRange, float InteractionScanRate)
{
	UAbilityTask_GrantNearbyInteraction* MyObj = NewAbilityTask<UAbilityTask_GrantNearbyInteraction>(OwningAbility);
	MyObj->InteractionScanRange = InteractionScanRange;
	MyObj->InteractionScanRate = InteractionScanRate;
	MyObj->TraceChannel = TraceChannel;
	return MyObj;
}

void UAbilityTask_GrantNearbyInteraction::QueryInteractables()
{
	UWorld* World = GetWorld();
	AActor* ActorOwner = GetAvatarActor();

	if (World && ActorOwner)
	{
		FCollisionQueryParams Params(SCENE_QUERY_STAT(UAbilityTask_GrantNearbyInteraction), false);

		TArray<FOverlapResult> OverlapResults;
		World->OverlapMultiByChannel(OUT OverlapResults, ActorOwner->GetActorLocation(), FQuat::Identity, TraceChannel, FCollisionShape::MakeSphere(InteractionScanRange), Params);

		if (OverlapResults.Num() > 0)
		{
			TArray<TScriptInterface<IInteractableTargetInterface>> InteractableTargets;
			UInteractionHelper::GetInteractableTargetsFromOverlapResults(OverlapResults, OUT InteractableTargets);

			FInteractionQuery InteractionQuery;
			InteractionQuery.RequestingAvatar = ActorOwner;
			InteractionQuery.RequestingController = Cast<AController>(ActorOwner->GetOwner());

			TArray<FInteractionOption> Options;
			for (TScriptInterface<IInteractableTargetInterface>& InteractiveTarget : InteractableTargets)
			{
				FInteractionOptionBuilder InteractionBuilder(InteractiveTarget, Options);
				InteractiveTarget->GatherInteractionOptions(InteractionQuery, InteractionBuilder);
			}

			// 检查是否有任何选项需要先赋予用户该能力才能使用。
			for (FInteractionOption& Option : Options)
			{
				if (Option.InteractionAbilityToGrant)
				{
					// 把能力赋予GAS，否则它就无法进行那种互动。
					FObjectKey ObjectKey(Option.InteractionAbilityToGrant);
					if (!InteractionAbilityCache.Find(ObjectKey))
					{
						FGameplayAbilitySpec Spec(Option.InteractionAbilityToGrant, 1, INDEX_NONE, this);
						FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->GiveAbility(Spec);
						InteractionAbilityCache.Add(ObjectKey, Handle);
					}
				}
			}
		}
	}
}
