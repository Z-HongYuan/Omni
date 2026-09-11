// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFramework/GameState.h"

#include "ModularGameState.generated.h"

#define UE_API MODULARGAMEPLAYACTORS_API

class UObject;

/** 与 AModularGameModeBase 配套使用，提供组件扩展的接收器生命周期。 */
UCLASS(MinimalAPI, Blueprintable)
class AModularGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	//~AActor 接口
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~AActor 接口结束
};


/** 与 AModularGameMode 配套使用，并向游戏状态组件转发比赛开始与结束事件。 */
UCLASS(MinimalAPI, Blueprintable)
class AModularGameState : public AGameState
{
	GENERATED_BODY()

public:
	//~AActor 接口
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~AActor 接口结束

protected:
	//~AGameState 接口
	UE_API virtual void HandleMatchHasStarted() override;
	UE_API virtual void HandleMatchHasEnded() override;
	//~AGameState 接口结束
};

#undef UE_API
