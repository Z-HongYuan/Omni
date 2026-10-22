// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ExtPlayerController.h"
#include "OmniPlayerController.generated.h"

#define UE_API OMNIGAME_API

class UExtAbilitySystemComponent;

/**
 * 项目玩家控制器，沿用父类的模块化组件接入及 LocalPlayer 状态通知。
 * PostProcessInput 处理 PlayerState ASC 的本帧能力输入；解除控制前清空本 Pawn 的 Avatar。
 * 完整 ASC 清理与注销通知由 PawnExtension 负责。
 */
UCLASS(MinimalAPI, Config = Game)
class AOmniPlayerController : public AExtPlayerController
{
	GENERATED_BODY()

public:
	UE_API AOmniPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API UExtAbilitySystemComponent* GetASC() const;

	UE_API virtual void ReceivedPlayer() override;
	UE_API virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;

protected:
	UE_API virtual void OnUnPossess() override;
};

#undef UE_API
