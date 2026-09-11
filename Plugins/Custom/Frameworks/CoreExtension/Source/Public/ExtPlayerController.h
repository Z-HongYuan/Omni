// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ModularPlayerController.h"
#include "ExtPlayerController.generated.h"

#define UE_API COREEXTENSION_API

/**
 * 转发游戏流程(事件)到 CustomLocalPlayer
 */
UCLASS(MinimalAPI, Config = Game)
class AExtPlayerController : public AModularPlayerController
{
	GENERATED_BODY()

public:
	UE_API AExtPlayerController(const FObjectInitializer& ObjectInitializer);

	UE_API virtual void ReceivedPlayer() override;
	UE_API virtual void SetPawn(APawn* InPawn) override;
	UE_API virtual void OnRep_PlayerState() override;

protected:
	UE_API virtual void OnPossess(class APawn* APawn) override;
	UE_API virtual void OnUnPossess() override;
};

#undef UE_API
