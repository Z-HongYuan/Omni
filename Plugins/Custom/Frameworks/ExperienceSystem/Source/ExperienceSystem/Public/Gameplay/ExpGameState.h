// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ModularGameState.h"
#include "ExpGameState.generated.h"

#define UE_API EXPERIENCESYSTEM_API

class UExpManagerComponent;

/** 持有体验管理组件的模块化 GameState。 */
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
