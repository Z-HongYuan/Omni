// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ModularAIController.h"
#include "ExpAIController.generated.h"

#define UE_API EXPERIENCESYSTEM_API

/** 体验系统的 AI 控制器，复用 PlayerState 的 PawnData、ASC 与 GameMode 重生流程。 */
UCLASS(MinimalAPI)
class AExpAIController : public AModularAIController
{
	GENERATED_BODY()

public:
	UE_API AExpAIController(const FObjectInitializer& ObjectInitializer);

	/** 仅服务器尝试重生 Inactive 或 Spectating 状态的控制器；通过 ControllerCanRestart 后交给 GameMode。 */
	UFUNCTION(BlueprintCallable, Category = "Experience")
	UE_API void ServerRestartController();
};

#undef UE_API
