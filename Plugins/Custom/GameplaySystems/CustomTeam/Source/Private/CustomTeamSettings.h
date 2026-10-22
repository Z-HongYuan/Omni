// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "CustomTeamSettings.generated.h"

#define UE_API CUSTOMTEAM_API

class UTeamDamageRuleBase;

/** 队伍系统全局设置，供子系统加载基础伤害规则和读取默认伤害策略。 */
UCLASS(MinimalAPI, Config=Game, DefaultConfig, meta=(DisplayName="Custom Team"))
class UCustomTeamSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** 返回 Project Settings 中的设置分类。 */
	virtual FName GetCategoryName() const override { return TEXT("ZHYS"); }

	/** 是否注册内置自伤与队伍关系规则；关闭后仅使用 GlobalRules 和默认伤害策略。 */
	UPROPERTY(EditAnywhere, Config, Category="TeamDamage")
	bool bUseDefaultRules = false;

	/** 没有规则明确放行或否决时，是否允许伤害。 */
	UPROPERTY(EditAnywhere, Config, Category="TeamDamage")
	bool bAllowDefaultDamage = false;

	/** 子系统初始化时加载并注册的全局伤害规则类，支持蓝图子类。 */
	UPROPERTY(EditAnywhere, Config, Category="TeamDamage")
	TArray<TSoftClassPtr<UTeamDamageRuleBase>> GlobalRules;
};
#undef UE_API
