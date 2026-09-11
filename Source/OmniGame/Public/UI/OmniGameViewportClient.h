// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "CommonGameViewportClient.h"
#include "OmniGameViewportClient.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目游戏视口客户端，键盘、轴和触摸输入路由由 CommonUI 父类提供。
 * 在 DefaultEngine.ini 中通过 GameViewportClientClassName 指定此类。
 *
 * 对照 Lyra 5.8，后续按需评估迁入：
 * - 根据 Platform.Trait.Input.HardwareCursor 选择软/硬件光标；当前沿用引擎默认行为。
 */
UCLASS(MinimalAPI)
class UOmniGameViewportClient : public UCommonGameViewportClient
{
	GENERATED_BODY()

public:
	UE_API virtual void Init(FWorldContext& WorldContext, UGameInstance* OwningGameInstance, bool bCreateNewAudioDevice = true) override;
};

#undef UE_API
