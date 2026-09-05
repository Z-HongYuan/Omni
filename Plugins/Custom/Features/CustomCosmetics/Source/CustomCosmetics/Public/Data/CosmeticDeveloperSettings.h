// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "CosmeticDataTypes.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "CosmeticDeveloperSettings.generated.h"

#define UE_API CUSTOMCOSMETICS_API

enum class ECosmeticCheatMode;

/**
 * 可由控制台变量控制的开发者设置
 * 用于在 外观系统 中自动添加测试 外观部件
 */
UCLASS(Config=Game, MinimalAPI)
class UCosmeticDeveloperSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	UCosmeticDeveloperSettings();

	/** 返回设置类别名称（在 Project Settings 中的路径） */
	virtual FName GetCategoryName() const override { return TEXT("ZHYS"); }

	// 不需要 config 每次启动时自己设置 外观添加数组
	UPROPERTY(Transient, EditAnywhere)
	TArray<FCharacterPart> CheatCharacterParts;

	// 不需要 config 启动时自己设置 外观添加模式
	UPROPERTY(Transient, EditAnywhere)
	ECosmeticCheatMode CheatMode;
};

#undef UE_API
