// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ExtGameInstance.h"
#include "OmniGameInstance.generated.h"

#define UE_API OMNIGAME_API

/** 跨地图存在的项目游戏实例，通用用户与会话初始化由插件父类负责。 */
UCLASS(MinimalAPI, Blueprintable, Config = Game)
class UOmniGameInstance : public UExtGameInstance
{
	GENERATED_BODY()

public:
	UE_API UOmniGameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API virtual void Init() override;
	UE_API virtual void Shutdown() override;
};

#undef UE_API
