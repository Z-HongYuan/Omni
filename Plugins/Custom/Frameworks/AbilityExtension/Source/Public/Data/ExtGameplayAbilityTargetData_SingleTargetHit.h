// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Abilities/GameplayAbilityTargetTypes.h"
#include "ExtGameplayAbilityTargetData_SingleTargetHit.generated.h"


/** 单目标命中数据的扩展类型，当前沿用父类的上下文写入与网络序列化。 */
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
		/** 为 FGameplayAbilityTargetDataHandle 提供自定义类型的网络序列化入口。 */
		WithNetSerializer = true
	};
};
