// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Core/TeamSubsystem.h"	// ETeamComparison
#include "TeamDamageRuleBase.generated.h"

#define UE_API CUSTOMTEAM_API

/** 单条伤害规则的判定结果，Block 优先于其他规则的 Allow。 */
UENUM(BlueprintType)
enum class ETeamDamageRuleResult : uint8
{
	/** 弃权，继续评估其他规则。 */
	Continue,

	/** 允许伤害，仍可被其他规则否决。 */
	Allow,

	/** 禁止伤害，直接否决本次判定。 */
	Block,
};

/** 子系统为一次伤害判定准备的只读上下文，包含双方对象、队伍关系和玩家关系。 */
USTRUCT(BlueprintType)
struct FTeamDamageRuleContext
{
	GENERATED_BODY()

	/** 本次伤害的发起对象。 */
	UPROPERTY(BlueprintReadOnly)
	const UObject* Instigator = nullptr;

	/** 本次伤害的目标对象。 */
	UPROPERTY(BlueprintReadOnly)
	const UObject* Target = nullptr;

	/** 由队伍子系统预先计算的双方队伍关系。 */
	UPROPERTY(BlueprintReadOnly)
	ETeamComparison Relationship = ETeamComparison::InvalidArgument;

	/** 子系统提供的同一玩家判定结果，供自伤等规则使用。 */
	UPROPERTY(BlueprintReadOnly)
	bool bSamePlayer = false;
};

/**
 * 可由 C++ 或蓝图扩展的伤害规则，由队伍子系统持有并综合判定。
 * 任一 Block 都会否决；否则 Allow 或 bAllowDefaultDamage 可放行伤害。
 * C++ 重写 EvaluateDamage_Implementation，蓝图重写 EvaluateDamage 事件。
 */
UCLASS(Blueprintable, Abstract)
class UTeamDamageRuleBase : public UObject
{
	GENERATED_BODY()

public:
	/** 是否启用此规则；关闭时不参与伤害判定。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TeamDamage")
	bool bEnabled = true;

	/** 供调试和显示使用的规则名称。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TeamDamage")
	FText RuleName;

	/** 评估当前上下文；返回 Allow 或 Block 表态，返回 Continue 弃权。 */
	UFUNCTION(BlueprintNativeEvent, Category="TeamDamage")
	ETeamDamageRuleResult EvaluateDamage(const FTeamDamageRuleContext& Context) const;
	UE_API virtual ETeamDamageRuleResult EvaluateDamage_Implementation(const FTeamDamageRuleContext& Context) const;
};

#undef UE_API
