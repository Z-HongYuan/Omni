// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "CommonGameViewportClient.h"
#include "OmniGameViewportClient.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目游戏视口客户端，键盘、轴和触摸输入路由由 CommonUI 父类提供。
 * 通过 DefaultEngine.ini 的 GameViewportClientClassName 配置。
 */
UCLASS(MinimalAPI)
class UOmniGameViewportClient : public UCommonGameViewportClient
{
	GENERATED_BODY()

public:
	UE_API virtual void Init(FWorldContext& WorldContext, UGameInstance* OwningGameInstance, bool bCreateNewAudioDevice = true) override;
};

#undef UE_API
