// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFramework/HUD.h"
#include "ModularHUD.generated.h"

#define UE_API MODULARGAMEPLAYACTORS_API

/**
 * 模块化的 HUD 支持 Component 增删
 */
UCLASS(MinimalAPI)
class AModularHUD : public AHUD
{
	GENERATED_BODY()

public:
	//~UObject interface
	UE_API virtual void PreInitializeComponents() override;
	//~End of UObject interface

protected:
	//~AActor interface
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface
};
#undef UE_API
