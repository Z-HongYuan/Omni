// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "Attributes/ExtAttributeSet.h"
#include "ExtHealthSet.generated.h"

#define UE_API ABILITYEXTENSION_API

/**
 * 最小生命属性集，伤害与治疗在结算后归零，生命值由服务器复制。
 * 对照 Lyra：保留四个生命属性及范围约束；允许简单 GE 直接修改 MetaDamage/MetaHealing。
 * 后续按需添加伤害 Execution、免疫与自毁规则、伤害消息。
 */
UCLASS(MinimalAPI, BlueprintType)
class UExtHealthSet : public UExtAttributeSet
{
	GENERATED_BODY()

public:
	UE_API UExtHealthSet();
	UE_API virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	ATTRIBUTE_ACCESSORS(UExtHealthSet, Health)
	ATTRIBUTE_ACCESSORS(UExtHealthSet, MaxHealth)
	ATTRIBUTE_ACCESSORS(UExtHealthSet, MetaDamage)
	ATTRIBUTE_ACCESSORS(UExtHealthSet, MetaHealing)

	UE_API virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	UE_API virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	UE_API virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	UE_API virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

private:
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "AbilityExtension|Health", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Health;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "AbilityExtension|Health", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MaxHealth;

	// 元属性：仅用于本次结算，消费后归零；不复制，不保存累计伤害或累计治疗。
	UPROPERTY(BlueprintReadOnly, Category = "AbilityExtension|Health", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MetaDamage;

	UPROPERTY(BlueprintReadOnly, Category = "AbilityExtension|Health", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MetaHealing;
};

#undef UE_API
