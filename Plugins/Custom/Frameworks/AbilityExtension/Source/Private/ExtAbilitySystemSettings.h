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

	// 通用伤害 GE：通过 SetByCaller 传入本次伤害量，修改 MetaDamage。
	UPROPERTY(EditAnywhere, Config, Category = "AbilitySystem")
	TSubclassOf<UGameplayEffect> DamageGameplayEffect_SetByCaller;

	// 通用治疗 GE：通过 SetByCaller 传入本次治疗量，修改 MetaHealing。
	UPROPERTY(EditAnywhere, Config, Category = "AbilitySystem")
	TSubclassOf<UGameplayEffect> HealGameplayEffect_SetByCaller;
};

#undef UE_API
