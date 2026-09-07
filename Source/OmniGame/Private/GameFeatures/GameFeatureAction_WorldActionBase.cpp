// Copyright © 2026 张鸿源. All Rights Reserved.

#include "GameFeatures/GameFeatureAction_WorldActionBase.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_WorldActionBase)

void UGameFeatureAction_WorldActionBase::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	GameInstanceStartHandles.FindOrAdd(Context) = FWorldDelegates::OnStartGameInstance.AddUObject(this,
		&UGameFeatureAction_WorldActionBase::HandleGameInstanceStart, FGameFeatureStateChangeContext(Context));

	// 对任何已经初始化了关联 GameInstance 的世界立即生效
	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		if (Context.ShouldApplyToWorldContext(WorldContext))
		{
			AddToWorld(WorldContext, Context);
		}
	}
}

void UGameFeatureAction_WorldActionBase::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	if (FDelegateHandle* FoundHandle = GameInstanceStartHandles.Find(Context))
	{
		FWorldDelegates::OnStartGameInstance.Remove(*FoundHandle);
		GameInstanceStartHandles.Remove(Context);
	}
}

void UGameFeatureAction_WorldActionBase::HandleGameInstanceStart(UGameInstance* GameInstance, FGameFeatureStateChangeContext ChangeContext)
{
	if (FWorldContext* WorldContext = GameInstance->GetWorldContext())
	{
		if (ChangeContext.ShouldApplyToWorldContext(*WorldContext))
		{
			AddToWorld(*WorldContext, ChangeContext);
		}
	}
}
