// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "ExpSubsystem.generated.h"

#define UE_API EXPERIENCESYSTEM_API

/** 向 GameFrameworkComponentManager 注册项目组件初始化状态链。 */
UCLASS(MinimalAPI)
class UExpSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	void InitComponentStateChain();
};

#undef UE_API
