// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "ExperienceSystemSettings.generated.h"

#define UE_API EXPERIENCESYSTEM_API

/**
 * 提供给Experience系统使用的开发者设置
 * 其中体验资产的优先级比WorldSetting高
 */
UCLASS(MinimalAPI, Config = Game)
class UExperienceSystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** 返回设置类别名称（在 Project Settings 中的路径） */
	virtual FName GetCategoryName() const override { return TEXT("ZHYS"); }

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, meta=(AllowedTypes="ExperienceDefinition"))
	FPrimaryAssetId ExperienceOverride;
};
#undef UE_API
