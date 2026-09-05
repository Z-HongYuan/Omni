// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPtr.h"
#include "AdvancedUISettings.generated.h"

#define UE_API ADVANCEDUI_API

class UDialogWidgetBase;
class UUIPolicy;

/**
 * 
 */
UCLASS(MinimalAPI, Config=Game, DefaultConfig, meta=(DisplayName="Advanced UI Settings"))
class UAdvancedUISettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** 返回设置类别名称（在 Project Settings 中的路径） */
	virtual FName GetCategoryName() const override { return TEXT("ZHYS"); }

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "UI Settings")
	TSoftClassPtr<UUIPolicy> DefaultUIPolicyClass;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Dialog Settings", meta=(AllowEditInlineCustomization))
	TMap<FGameplayTag, TSoftClassPtr<UDialogWidgetBase>> DialogSoftClasses;
};

#undef UE_API
