// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ModularGameState.h"
#include "ExpGameState.generated.h"

#define UE_API EXPERIENCESYSTEM_API

class UExpManagerComponent;

/**
 * 模块化基础游戏状态类
 * 支持 体验管理组件
 */
UCLASS(MinimalAPI)
class AExpGameState : public AModularGameStateBase
{
	GENERATED_BODY()

public:
	UE_API AExpGameState(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY()
	TObjectPtr<UExpManagerComponent> ExperienceManagerComponent;
};
#undef UE_API
