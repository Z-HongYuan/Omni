// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ExtensionPlayerController.h"
#include "OmniPlayerController.generated.h"

#define UE_API OMNIGAME_API

class UCustomAbilitySystemComponent;

/**
 * 项目使用的默认玩家控制器类
 */
UCLASS(MinimalAPI)
class AOmniPlayerController : public AExtensionPlayerController
{
	GENERATED_BODY()

public:
	UE_API AOmniPlayerController(const FObjectInitializer& ObjectInitializer);

	// 获取玩家状态上持有的 ASC，玩家状态未就绪时返回空
	UE_API UCustomAbilitySystemComponent* GetCustomAbilitySystemComponent() const;

	// 每帧把积累的能力输入交给 ASC 处理
	UE_API virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;

protected:
};
#undef UE_API
