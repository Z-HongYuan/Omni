// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "CosmeticDataTypes.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "CosmeticDeveloperSettings.generated.h"

#define UE_API CUSTOMCOSMETICS_API

enum class ECosmeticCheatMode;

/** 外观调试设置，供控制器组件添加或替换测试部件。 */
UCLASS(Config=Game, MinimalAPI)
class UCosmeticDeveloperSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	UCosmeticDeveloperSettings();

	/** 返回 Project Settings 中的设置分类。 */
	virtual FName GetCategoryName() const override { return TEXT("ZHYS"); }

	/** 本次会话使用的测试外观部件，不写入配置。 */
	UPROPERTY(Transient, EditAnywhere)
	TArray<FCharacterPart> CheatCharacterParts;

	/** 本次会话使用的测试外观模式，不写入配置。 */
	UPROPERTY(Transient, EditAnywhere)
	ECosmeticCheatMode CheatMode;
};

#undef UE_API
