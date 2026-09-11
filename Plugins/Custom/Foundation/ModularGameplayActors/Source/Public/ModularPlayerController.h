// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFramework/PlayerController.h"

#include "ModularPlayerController.generated.h"

#define UE_API MODULARGAMEPLAYACTORS_API

class UObject;

/**
 * 模块化玩家控制器，在 ReceivedPlayer 时发送 GameActorReady，并向控制器组件转发玩家事件。
 * 此处的就绪表示控制器已经关联玩家，具体玩法数据的初始化由上层系统负责。
 */
UCLASS(MinimalAPI, Blueprintable)
class AModularPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	//~AActor 接口
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~AActor 接口结束

	//~APlayerController 接口
	UE_API virtual void ReceivedPlayer() override;
	UE_API virtual void PlayerTick(float DeltaTime) override;
	//~APlayerController 接口结束
};

#undef UE_API
