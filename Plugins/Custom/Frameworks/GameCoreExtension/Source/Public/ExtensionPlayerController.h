// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ModularPlayerController.h"
#include "ExtensionPlayerController.generated.h"

#define UE_API GAMECOREEXTENSION_API

/**
 * 转发游戏流程(事件)到 CustomLocalPlayer
 */
UCLASS(MinimalAPI, Config = Game)
class AExtensionPlayerController : public AModularPlayerController
{
	GENERATED_BODY()

public:
	UE_API AExtensionPlayerController(const FObjectInitializer& ObjectInitializer);

	UE_API virtual void ReceivedPlayer() override;
	UE_API virtual void SetPawn(APawn* InPawn) override;
	UE_API virtual void OnRep_PlayerState() override;

protected:
	UE_API virtual void OnPossess(class APawn* APawn) override;
	UE_API virtual void OnUnPossess() override;
};

#undef UE_API
