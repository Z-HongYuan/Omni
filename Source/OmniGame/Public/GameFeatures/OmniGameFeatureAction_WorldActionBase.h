// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFeatureAction.h"
#include "GameFeaturesSubsystem.h"
#include "OmniGameFeatureAction_WorldActionBase.generated.h"

#define UE_API OMNIGAME_API

/** 将项目 GF 动作应用到匹配的游戏世界，兼容激活后创建的 GameInstance。 */
UCLASS(Abstract, MinimalAPI)
class UOmniGameFeatureAction_WorldActionBase : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	UE_API virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	UE_API virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;

protected:
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) PURE_VIRTUAL(UOmniGameFeatureAction_WorldActionBase::AddToWorld,);

private:
	void HandleGameInstanceStart(UGameInstance* GameInstance, FGameFeatureStateChangeContext ChangeContext);
	TMap<FGameFeatureStateChangeContext, FDelegateHandle> GameInstanceStartHandles;
};

#undef UE_API
