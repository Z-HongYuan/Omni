// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ExtHealthLibrary.generated.h"

class UAbilitySystemComponent;

/** 消费开发者设置中的通用 GE 和 SetByCaller Tag。伤害判定、阵营、无敌和死亡规则由调用方负责。 */
UCLASS()
class ABILITYEXTENSION_API UExtHealthLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 仅服务端应用有限正数；返回是否成功应用，Instant GE 不要求持久句柄有效。
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "AbilityExtension|Health")
	static bool ApplyDamage(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target, float Amount, const FHitResult& Hit);
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "AbilityExtension|Health")
	static bool ApplyHealing(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target, float Amount);
};
