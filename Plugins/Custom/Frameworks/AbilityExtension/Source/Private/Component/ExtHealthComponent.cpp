// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Component/ExtHealthComponent.h"

#include "Attributes/ExtHealthSet.h"
#include "GameplayEffectExtension.h"
#include "LogAbilityExtension.h"
#include "System/ExtAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExtHealthComponent)

UExtHealthComponent::UExtHealthComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

UExtHealthComponent* UExtHealthComponent::FindHealthComponent(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<UExtHealthComponent>() : nullptr;
}

void UExtHealthComponent::InitializeWithAbilitySystem(UExtAbilitySystemComponent* InASC)
{
	// 重复初始化不重复绑定；换 ASC 前先移除旧委托。
	if (AbilitySystemComponent == InASC && HealthSet) return;
	UninitializeFromAbilitySystem();

	if (!IsValid(InASC))
	{
		UE_LOG(LogAbilityExtension, Warning, TEXT("HealthComponent on %s requires a valid ASC."), *GetNameSafe(GetOwner()));
		return;
	}

	const UExtHealthSet* InHealthSet = InASC->GetSet<UExtHealthSet>();
	if (!InHealthSet)
	{
		UE_LOG(LogAbilityExtension, Warning, TEXT("HealthComponent on %s cannot find ExtHealthSet on %s."), *GetNameSafe(GetOwner()), *GetNameSafe(InASC));
		return;
	}

	AbilitySystemComponent = InASC;
	HealthSet = InHealthSet;
	HealthChangedHandle = InASC->GetGameplayAttributeValueChangeDelegate(UExtHealthSet::GetHealthAttribute()).AddUObject(this, &ThisClass::HandleHealthChanged);
	MaxHealthChangedHandle = InASC->GetGameplayAttributeValueChangeDelegate(UExtHealthSet::GetMaxHealthAttribute()).AddUObject(this, &ThisClass::HandleMaxHealthChanged);

	// 首次推送当前值供 UI 初始化，绑定本身不修改生命属性。
	OnHealthChanged.Broadcast(this, GetHealth(), GetHealth(), nullptr);
	OnMaxHealthChanged.Broadcast(this, GetMaxHealth(), GetMaxHealth(), nullptr);
}

void UExtHealthComponent::UninitializeFromAbilitySystem()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UExtHealthSet::GetHealthAttribute()).Remove(HealthChangedHandle);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UExtHealthSet::GetMaxHealthAttribute()).Remove(MaxHealthChangedHandle);
	}

	HealthChangedHandle.Reset();
	MaxHealthChangedHandle.Reset();
	HealthSet = nullptr;
	AbilitySystemComponent = nullptr;
}

float UExtHealthComponent::GetHealth() const
{
	return HealthSet ? HealthSet->GetHealth() : 0.0f;
}

float UExtHealthComponent::GetMaxHealth() const
{
	return HealthSet ? HealthSet->GetMaxHealth() : 0.0f;
}

float UExtHealthComponent::GetHealthNormalized() const
{
	const float MaxHealth = GetMaxHealth();
	return MaxHealth > 0.0f ? GetHealth() / MaxHealth : 0.0f;
}

void UExtHealthComponent::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	if (Data.OldValue == Data.NewValue) return;

	// 客户端复制或直接修改属性时，通常没有效果来源上下文。
	AActor* Instigator = Data.GEModData ? Data.GEModData->EffectSpec.GetEffectContext().GetOriginalInstigator() : nullptr;
	OnHealthChanged.Broadcast(this, Data.OldValue, Data.NewValue, Instigator);

	// 只通知正数到零的变化；归零后的重复伤害不重复通知。
	const bool bOutOfHealth = Data.OldValue > 0.0f && Data.NewValue <= 0.0f;
	if (bOutOfHealth && AbilitySystemComponent && GetHealth() <= 0.0f && GetOwner()->HasAuthority())
	{
		OnOutOfHealth.Broadcast(this, Data.OldValue, Data.NewValue, Instigator);
	}
}

void UExtHealthComponent::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	if (Data.OldValue == Data.NewValue) return;

	AActor* Instigator = Data.GEModData ? Data.GEModData->EffectSpec.GetEffectContext().GetOriginalInstigator() : nullptr;
	OnMaxHealthChanged.Broadcast(this, Data.OldValue, Data.NewValue, Instigator);
}

void UExtHealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UninitializeFromAbilitySystem();
	Super::EndPlay(EndPlayReason);
}

void UExtHealthComponent::OnUnregister()
{
	UninitializeFromAbilitySystem();
	Super::OnUnregister();
}
