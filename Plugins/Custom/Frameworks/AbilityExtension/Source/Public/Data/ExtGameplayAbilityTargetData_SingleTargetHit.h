// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Abilities/GameplayAbilityTargetTypes.h"
#include "ExtGameplayAbilityTargetData_SingleTargetHit.generated.h"


/*
 * 针对单一目标命中追踪的游戏专属新增内容
 * 模版修改
 * 最基础的 FGameplayAbilityTargetData_SingleTargetHit 继承拓展方法
 */
USTRUCT()
struct FExtGameplayAbilityTargetData_SingleTargetHit : public FGameplayAbilityTargetData_SingleTargetHit
{
	GENERATED_BODY()

	FExtGameplayAbilityTargetData_SingleTargetHit()
	// : CartridgeID(-1)
	{
	}

	virtual void AddTargetDataToContext(FGameplayEffectContextHandle& Context, bool bIncludeActorArray) const override;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	virtual UScriptStruct* GetScriptStruct() const override;

	// /** 识别允许识别同一弹药中的多个子弹 */
	// UPROPERTY()
	// int32 CartridgeID;
};

template <>
struct TStructOpsTypeTraits<FExtGameplayAbilityTargetData_SingleTargetHit> : public TStructOpsTypeTraitsBase2<FExtGameplayAbilityTargetData_SingleTargetHit>
{
	enum
	{
		WithNetSerializer = true // 目前，FGameplayAbilityTargetDataHandle 网络序列化工作是必须的
	};
};
