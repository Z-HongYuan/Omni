// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFramework/Pawn.h"

#include "ModularPawn.generated.h"

#define UE_API MODULARGAMEPLAYACTORS_API

class UObject;

/** 支持游戏扩展功能插件的最小类 */
UCLASS(MinimalAPI, Blueprintable)
class AModularPawn : public APawn
{
	GENERATED_BODY()

public:
	//~ Begin AActor interface
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor interface
};

#undef UE_API
