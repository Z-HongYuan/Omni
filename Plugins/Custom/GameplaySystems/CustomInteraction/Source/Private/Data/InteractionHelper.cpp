// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Data/InteractionHelper.h"
#include "Components/PrimitiveComponent.h"
#include "Core/InteractableTargetInterface.h"
#include "Engine/OverlapResult.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractionHelper)

AActor* UInteractionHelper::GetActorFromInteractableTarget(TScriptInterface<IInteractableTargetInterface> InteractableTarget)
{
	if (UObject* Object = InteractableTarget.GetObject())
	{
		if (AActor* Actor = Cast<AActor>(Object))
		{
			return Actor;
		}
		else if (UActorComponent* ActorComponent = Cast<UActorComponent>(Object))
		{
			return ActorComponent->GetOwner();
		}
		else
		{
			// 当前仅支持 Actor 或 ActorComponent 作为交互目标。
			unimplemented();
		}
	}

	return nullptr;
}

void UInteractionHelper::GetInteractableTargetsFromActor(AActor* Actor, TArray<TScriptInterface<IInteractableTargetInterface>>& OutInteractableTargets)
{
	// 先收集 Actor 自身提供的交互接口。
	TScriptInterface<IInteractableTargetInterface> InteractableActor(Actor);
	if (InteractableActor)
	{
		OutInteractableTargets.Add(InteractableActor);
	}

	// 继续收集组件接口；Actor 已实现接口时也不会跳过组件。
	TArray<UActorComponent*> InteractableComponents = TArray<UActorComponent*>();
	if (Actor)
	{
		InteractableComponents = Actor->GetComponentsByInterface(UInteractableTargetInterface::StaticClass());
	}
	for (UActorComponent* InteractableComponent : InteractableComponents)
	{
		OutInteractableTargets.Add(TScriptInterface<IInteractableTargetInterface>(InteractableComponent));
	}
}

void UInteractionHelper::GetInteractableTargetsFromOverlapResults(const TArray<FOverlapResult>& OverlapResults, TArray<TScriptInterface<IInteractableTargetInterface>>& OutInteractableTargets)
{
	for (const FOverlapResult& Overlap : OverlapResults)
	{
		TScriptInterface<IInteractableTargetInterface> InteractableActor(Overlap.GetActor());
		if (InteractableActor)
		{
			OutInteractableTargets.AddUnique(InteractableActor);
		}

		TScriptInterface<IInteractableTargetInterface> InteractableComponent(Overlap.GetComponent());
		if (InteractableComponent)
		{
			OutInteractableTargets.AddUnique(InteractableComponent);
		}
	}
}

void UInteractionHelper::GetInteractableTargetsFromHitResult(const FHitResult& HitResult, TArray<TScriptInterface<IInteractableTargetInterface>>& OutInteractableTargets)
{
	TScriptInterface<IInteractableTargetInterface> InteractableActor(HitResult.GetActor());
	if (InteractableActor)
	{
		OutInteractableTargets.AddUnique(InteractableActor);
	}

	TScriptInterface<IInteractableTargetInterface> InteractableComponent(HitResult.GetComponent());
	if (InteractableComponent)
	{
		OutInteractableTargets.AddUnique(InteractableComponent);
	}
}
