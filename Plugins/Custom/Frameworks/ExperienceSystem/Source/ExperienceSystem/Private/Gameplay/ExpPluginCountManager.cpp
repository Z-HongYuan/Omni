// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Gameplay/ExpPluginCountManager.h"

#include "Engine/Engine.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExpPluginCountManager)

#if WITH_EDITOR

void UExpPluginCountManager::OnPlayInEditorBegun()
{
	ensure(GameFeaturePluginRequestCountMap.IsEmpty());
	GameFeaturePluginRequestCountMap.Empty();
}

void UExpPluginCountManager::NotifyOfPluginActivation(const FString& PluginURL)
{
	if (GIsEditor)
	{
		UExpPluginCountManager* PluginCountManager = GEngine->GetEngineSubsystem<UExpPluginCountManager>();
		check(PluginCountManager);

		// 每个体验各记一次；并发的加载、激活请求仍交给 GameFeatures 处理。
		int32& Count = PluginCountManager->GameFeaturePluginRequestCountMap.FindOrAdd(PluginURL);
		++Count;
	}
}

bool UExpPluginCountManager::RequestToDeactivatePlugin(const FString& PluginURL)
{
	if (GIsEditor)
	{
		UExpPluginCountManager* PluginCountManager = GEngine->GetEngineSubsystem<UExpPluginCountManager>();
		check(PluginCountManager);

		int32& Count = PluginCountManager->GameFeaturePluginRequestCountMap.FindChecked(PluginURL);
		--Count;

		if (Count == 0)
		{
			PluginCountManager->GameFeaturePluginRequestCountMap.Remove(PluginURL);
			return true;
		}

		return false;
	}

	return true;
}

#endif
