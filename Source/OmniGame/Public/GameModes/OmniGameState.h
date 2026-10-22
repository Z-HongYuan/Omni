// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Gameplay/ExpGameState.h"
#include "OmniGameState.generated.h"

#define UE_API OMNIGAME_API

/** 项目对局状态，体验加载与模块化组件接入由父类管理。 */
UCLASS(MinimalAPI, Config = Game)
class AOmniGameState : public AExpGameState
{
	GENERATED_BODY()

public:
	UE_API AOmniGameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API virtual void PostInitializeComponents() override;
};

#undef UE_API
