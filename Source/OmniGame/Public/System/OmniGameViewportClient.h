// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "CommonGameViewportClient.h"
#include "OmniGameViewportClient.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目使用的视口客户端类
 */
UCLASS(MinimalAPI)
class UOmniGameViewportClient : public UCommonGameViewportClient
{
	GENERATED_BODY()

public:
	virtual void Init(struct FWorldContext& WorldContext, UGameInstance* OwningGameInstance, bool bCreateNewAudioDevice = true) override;
};

#undef UE_API
