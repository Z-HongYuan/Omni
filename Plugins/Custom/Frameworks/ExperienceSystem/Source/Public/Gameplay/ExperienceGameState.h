// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ModularGameState.h"
#include "ExperienceGameState.generated.h"

#define UE_API EXPERIENCESYSTEM_API

class UExperienceManagerComponent;

/**
 * 模块化基础游戏状态类
 * 支持 体验管理组件
 */
UCLASS(MinimalAPI)
class AExperienceGameState : public AModularGameStateBase
{
	GENERATED_BODY()

public:
	UE_API AExperienceGameState(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY()
	TObjectPtr<UExperienceManagerComponent> ExperienceManagerComponent;
};
#undef UE_API
