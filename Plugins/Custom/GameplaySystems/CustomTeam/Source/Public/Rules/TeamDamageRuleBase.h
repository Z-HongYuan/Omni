// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Core/TeamSubsystem.h"	// ETeamComparison
#include "TeamDamageRuleBase.generated.h"

#define UE_API CUSTOMTEAM_API

/**
 * 伤害规则判定结果
 * Continue: 规则不表态(弃权), 继续询问下一条规则
 * Allow:    放行票(允许伤害)
 * Block:    否决票(禁止伤害, 优先级最高)
 */
UENUM(BlueprintType)
enum class ETeamDamageRuleResult : uint8
{
	// 弃权, 继续下一条规则
	Continue,

	// 放行: 允许伤害
	Allow,

	// 否决: 禁止伤害(一票否决, 优先于 Allow)
	Block,
};

/**
 * 一次伤害判定的上下文(由 UTeamSubsystem 计算好, 传入每条规则)
 * 规则只读该结构即可完成判定, 无需再查询子系统。
 */
USTRUCT(BlueprintType)
struct FTeamDamageRuleContext
{
	GENERATED_BODY()

	// 伤害发起者
	UPROPERTY(BlueprintReadOnly)
	const UObject* Instigator = nullptr;

	// 伤害目标
	UPROPERTY(BlueprintReadOnly)
	const UObject* Target = nullptr;

	// 双方队伍关系(已由子系统算出)
	UPROPERTY(BlueprintReadOnly)
	ETeamComparison Relationship = ETeamComparison::InvalidArgument;

	// 发起者与目标是否属于同一玩家(自我豁免判定用, 已由子系统算出)
	UPROPERTY(BlueprintReadOnly)
	bool bSamePlayer = false;
};

/**
 * 伤害规则基类
 *
 * 插件只负责"托管规则数组 + 综合判定", 规则内容完全由项目填充:
 * - C++: 继承本类, 重载 EvaluateDamage_Implementation;
 * - 蓝图: 子类化本类, 配置 bEnabled/RuleName, 重载 EvaluateDamage 事件。
 *
 * 综合规则(在 UTeamSubsystem::CanCauseDamage 内固定):
 * 任何规则返回 Block → 禁止; 没有任何 Block 且至少一条 Allow → 允许; 全部弃权 → 禁止。
 */
UCLASS(Blueprintable, Abstract)
class UTeamDamageRuleBase : public UObject
{
	GENERATED_BODY()

public:
	// 规则开关(关闭的规则不参与判定)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TeamDamage")
	bool bEnabled = true;

	// 规则名称(调试/显示用)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TeamDamage")
	FText RuleName;

	/**
	 * 评估本次伤害。返回 Allow/Block 参与综合, 返回 Continue 表示弃权。
	 */
	UFUNCTION(BlueprintNativeEvent, Category="TeamDamage")
	ETeamDamageRuleResult EvaluateDamage(const FTeamDamageRuleContext& Context) const;
	UE_API virtual ETeamDamageRuleResult EvaluateDamage_Implementation(const FTeamDamageRuleContext& Context) const;
};

#undef UE_API
