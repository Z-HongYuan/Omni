// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "Templates/SubclassOf.h"
#include "ExtAbilitySystemSettings.generated.h"

#define  UE_API ABILITYEXTENSION_API

class UGameplayEffect;

/**
 * 自定义ASC的参数设置
 */
UCLASS(MinimalAPI, meta = (DisplayName = "Ability Extension Settings"), Config = Game)
class UExtAbilitySystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** 返回设置类别名称（在 Project Settings 中的路径） */
	virtual FName GetCategoryName() const override { return TEXT("ZHYS"); }

	UPROPERTY(EditAnywhere, Config, Category = "AbilitySystem")
	TSubclassOf<UGameplayEffect> DynamicTagGameplayEffect;
};

#undef UE_API
