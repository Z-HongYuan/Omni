// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPtr.h"
#include "GameUISettings.generated.h"

#define UE_API GAMEUI_API

class UDialogWidgetBase;
class UGameUIPolicy;

/** 配置默认 UI 策略，以及对话框标签到控件类的映射。 */
UCLASS(MinimalAPI, Config=Game, DefaultConfig, meta=(DisplayName="Game UI Settings"))
class UGameUISettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** 返回 Project Settings 中的设置类别。 */
	virtual FName GetCategoryName() const override { return TEXT("ZHYS"); }

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "UI Settings")
	TSoftClassPtr<UGameUIPolicy> DefaultUIPolicyClass;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Dialog Settings", meta=(AllowEditInlineCustomization))
	TMap<FGameplayTag, TSoftClassPtr<UDialogWidgetBase>> DialogSoftClasses;
};

#undef UE_API
