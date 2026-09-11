// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFramework/Pawn.h"

#include "ModularPawn.generated.h"

#define UE_API MODULARGAMEPLAYACTORS_API

class UObject;

/** 模块化 Pawn，负责组件接收器的注册、就绪通知与退出清理，具体玩法由组件扩展。 */
UCLASS(MinimalAPI, Blueprintable)
class AModularPawn : public APawn
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
