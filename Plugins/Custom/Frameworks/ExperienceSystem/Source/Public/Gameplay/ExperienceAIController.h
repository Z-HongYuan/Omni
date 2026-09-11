// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ModularAIController.h"
#include "ExperienceAIController.generated.h"

#define UE_API EXPERIENCESYSTEM_API

/**
 * 体验系统使用的 AI 控制器基类
 */
UCLASS(MinimalAPI)
class AExperienceAIController : public AModularAIController
{
	GENERATED_BODY()

public:
	UE_API AExperienceAIController(const FObjectInitializer& ObjectInitializer);

	// 尝试重启此控制器（例如重生）
	// 仅服务端生效，内部会先走 GameMode 的 ControllerCanRestart 判定，再交给 GameMode 统一重生
	// 与 APlayerController 的 ServerRestartPlayer_Implementation 是一对，由 GameMode 的 RequestPlayerRestartNextFrame 调用
	// 将 AIController 统一重启流程，与 APlayerController 保持一致
	UFUNCTION(BlueprintCallable, Category = "Experience")
	UE_API void ServerRestartController();
};

#undef UE_API
