// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "CustomTeamSettings.generated.h"

#define UE_API CUSTOMTEAM_API

class UTeamDamageRuleBase;

/**
 * CustomTeam 全局参数设置
 * 配置入口: Project Settings → Custom Team
 * 伤害规则等全局参数在此配置, 由 UTeamSubsystem 初始化时读取并生效。
 */
UCLASS(MinimalAPI, Config=Game, DefaultConfig, meta=(DisplayName="Custom Team"))
class UCustomTeamSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** 返回设置类别名称（在 Project Settings 中的路径） */
	virtual FName GetCategoryName() const override { return TEXT("ZHYS"); }

	// 是否注册插件内置默认伤害规则(自我豁免 + 队伍关系: 不同队允许/同队禁止)
	// 关闭后请自行在 GlobalRules 中配置等效规则, 否则默认"全部禁止"
	UPROPERTY(EditAnywhere, Config, Category="TeamDamage")
	bool bUseDefaultRules = false;

	// 在伤害规则中, 全部规则弃权是否允许伤害
	UPROPERTY(EditAnywhere, Config, Category="TeamDamage")
	bool bAllowDefaultDamage = false;

	// 全局基础伤害规则: 整个项目开局自动注册的规则类(支持蓝图子类)
	UPROPERTY(EditAnywhere, Config, Category="TeamDamage")
	TArray<TSoftClassPtr<UTeamDamageRuleBase>> GlobalRules;
};
#undef UE_API
