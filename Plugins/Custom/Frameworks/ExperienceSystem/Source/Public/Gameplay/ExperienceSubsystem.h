// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "ExperienceSubsystem.generated.h"

#define UE_API EXPERIENCESYSTEM_API

/**
 * 用于注册组件初始化链条
 */
UCLASS(MinimalAPI)
class UExperienceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	void InitComponentStateChain();
};

#undef UE_API
