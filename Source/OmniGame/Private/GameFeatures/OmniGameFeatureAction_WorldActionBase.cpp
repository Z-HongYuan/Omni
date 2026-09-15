// Copyright © 2026 张鸿源. All Rights Reserved.

#include "GameFeatures/OmniGameFeatureAction_WorldActionBase.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniGameFeatureAction_WorldActionBase)

void UOmniGameFeatureAction_WorldActionBase::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	Super::OnGameFeatureActivating(Context);
	FDelegateHandle& Handle = GameInstanceStartHandles.FindOrAdd(Context);
	FWorldDelegates::OnStartGameInstance.Remove(Handle);
	Handle = FWorldDelegates::OnStartGameInstance.AddUObject(this, &ThisClass::HandleGameInstanceStart, FGameFeatureStateChangeContext(Context));
	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		if (Context.ShouldApplyToWorldContext(WorldContext)) AddToWorld(WorldContext, Context);
	}
}

void UOmniGameFeatureAction_WorldActionBase::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	if (FDelegateHandle* Handle = GameInstanceStartHandles.Find(Context))
	{
		FWorldDelegates::OnStartGameInstance.Remove(*Handle);
		GameInstanceStartHandles.Remove(Context);
	}
	Super::OnGameFeatureDeactivating(Context);
}

void UOmniGameFeatureAction_WorldActionBase::HandleGameInstanceStart(UGameInstance* GameInstance, FGameFeatureStateChangeContext ChangeContext)
{
	if (const FWorldContext* WorldContext = GameInstance->GetWorldContext())
	{
		if (ChangeContext.ShouldApplyToWorldContext(*WorldContext)) AddToWorld(*WorldContext, ChangeContext);
	}
}
