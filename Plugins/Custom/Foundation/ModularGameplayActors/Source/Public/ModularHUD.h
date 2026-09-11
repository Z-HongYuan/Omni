// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFramework/HUD.h"

#include "ModularHUD.generated.h"

#define UE_API MODULARGAMEPLAYACTORS_API

/**
 * 模块化 HUD，负责组件接收器的注册、就绪通知与退出清理。
 * 扩展机制对应 LyraHUD；具体界面布局与调试绘制由上层实现。
 */
UCLASS(MinimalAPI, Blueprintable)
class AModularHUD : public AHUD
{
	GENERATED_BODY()

public:
	//~AActor 接口
	UE_API virtual void PreInitializeComponents() override;
	//~AActor 接口结束

protected:
	//~AActor 接口
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~AActor 接口结束
};

#undef UE_API
