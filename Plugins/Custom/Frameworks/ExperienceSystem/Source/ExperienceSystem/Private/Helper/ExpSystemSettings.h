// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "ExpSystemSettings.generated.h"

#define UE_API EXPERIENCESYSTEM_API

/** Experience 开发者设置；体验覆盖仅在 PIE 中读取，优先于命令行和世界设置，低于 URL 选项。 */
UCLASS(MinimalAPI, Config = Game)
class UExpSystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** 返回项目设置中的类别名称。 */
	virtual FName GetCategoryName() const override { return TEXT("ZHYS"); }

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, meta=(AllowedTypes="ExpDefinition"))
	FPrimaryAssetId ExperienceOverride;
};
#undef UE_API
