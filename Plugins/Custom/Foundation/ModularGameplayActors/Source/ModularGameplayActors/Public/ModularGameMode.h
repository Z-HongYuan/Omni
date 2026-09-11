// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFramework/GameMode.h"

#include "ModularGameMode.generated.h"

#define UE_API MODULARGAMEPLAYACTORS_API

class UObject;

/**
 * 与 AModularGameStateBase 配套使用，默认选用模块化控制器、玩家状态、Pawn 和 HUD。
 * 本类负责设置默认类型，不注册 GameMode 自身为组件接收器。
 */
UCLASS(MinimalAPI, Blueprintable)
class AModularGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	UE_API AModularGameModeBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

/** 与 AModularGameState 配套使用，提供带比赛状态机的模块化默认类型组合。 */
UCLASS(MinimalAPI, Blueprintable)
class AModularGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	UE_API AModularGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

#undef UE_API
