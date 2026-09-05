// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFramework/GameState.h"

#include "ModularGameState.generated.h"

#define UE_API MODULARGAMEPLAYACTORS_API

class UObject;

/** 模块化的 GameStateBase */
UCLASS(MinimalAPI, Blueprintable)
class AModularGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	//~ Begin AActor interface
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor interface
};


/** 模块化的 GameState */
UCLASS(MinimalAPI, Blueprintable)
class AModularGameState : public AGameState
{
	GENERATED_BODY()

public:
	//~ Begin AActor interface
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor interface

protected:
	//~ Begin AGameState interface
	UE_API virtual void HandleMatchHasStarted() override;
	UE_API virtual void HandleMatchHasEnded() override;
	//~ Begin AGameState interface
};

#undef UE_API
