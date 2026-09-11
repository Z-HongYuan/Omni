// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "AIController.h"

#include "ModularAIController.generated.h"

#define UE_API MODULARGAMEPLAYACTORS_API

class UObject;

/** 模块化 AI 控制器，负责注册组件接收器，并在父类 BeginPlay 前发送 GameActorReady。 */
UCLASS(MinimalAPI, Blueprintable)
class AModularAIController : public AAIController
{
	GENERATED_BODY()

public:
	//~AActor 接口
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~AActor 接口结束
};

#undef UE_API
