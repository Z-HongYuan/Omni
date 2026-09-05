// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "AIController.h"

#include "ModularAIController.generated.h"

#define UE_API MODULARGAMEPLAYACTORS_API

class UObject;

/** 支持游戏扩展功能插件的最小类 */
UCLASS(MinimalAPI, Blueprintable)
class AModularAIController : public AAIController
{
	GENERATED_BODY()

public:
	//~ Begin AActor Interface
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface
};

#undef UE_API
