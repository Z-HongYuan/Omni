// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFramework/GameMode.h"

#include "ModularGameMode.generated.h"

#define UE_API MODULARGAMEPLAYACTORS_API

class UObject;

/** 模块化的 GameModeBase */
UCLASS(MinimalAPI, Blueprintable)
class AModularGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	UE_API AModularGameModeBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

/** 模块化的 GameMode */
UCLASS(MinimalAPI, Blueprintable)
class AModularGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	UE_API AModularGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

#undef UE_API
