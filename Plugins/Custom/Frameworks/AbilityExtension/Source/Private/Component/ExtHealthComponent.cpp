// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Component/ExtHealthComponent.h"

#include "Attributes/ExtHealthSet.h"
#include "Data/ExtAbilitySystemTags.h"
#include "ExtAbilitySystemSettings.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "GameplayPrediction.h"
#include "LogAbilityExtension.h"
#include "Net/UnrealNetwork.h"
#include "System/ExtAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExtHealthComponent)

UExtHealthComponent::UExtHealthComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UExtHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, DeathState);
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
	OutOfHealthHandle = HealthSet->OnOutOfHealth.AddUObject(this, &ThisClass::HandleOutOfHealth);

	ClearGameplayTags();

	// 首次推送当前值供 UI 初始化，绑定本身不修改生命属性。
	OnHealthChanged.Broadcast(this, GetHealth(), GetHealth(), nullptr);
	OnMaxHealthChanged.Broadcast(this, GetMaxHealth(), GetMaxHealth(), nullptr);
}

void UExtHealthComponent::UninitializeFromAbilitySystem()
{
	ClearGameplayTags();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UExtHealthSet::GetHealthAttribute()).Remove(HealthChangedHandle);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UExtHealthSet::GetMaxHealthAttribute()).Remove(MaxHealthChangedHandle);
	}

	HealthChangedHandle.Reset();
	MaxHealthChangedHandle.Reset();

	if (HealthSet) HealthSet->OnOutOfHealth.Remove(OutOfHealthHandle);
	OutOfHealthHandle.Reset();

	HealthSet = nullptr;
	AbilitySystemComponent = nullptr;
}

bool UExtHealthComponent::InitializeHealthForSpawn()
{
	if (!GetOwner()->HasAuthority()
		|| bHasInitializedSpawnHealth
		|| IsDeadOrDying()
		|| !AbilitySystemComponent
		|| !HealthSet
		|| AbilitySystemComponent->GetAvatarActor() != GetOwner())
	{
		return false;
	}

	// 沿用 Lyra 的基础值设置方式，但由项目出生流程显式调用，不放进 ASC 绑定。
	// 先标记，避免属性通知中的重入再次初始化；已有 GE 的保留策略仍由玩法决定。
	bHasInitializedSpawnHealth = true;
	AbilitySystemComponent->SetNumericAttributeBase(UExtHealthSet::GetHealthAttribute(), GetMaxHealth());
	return true;
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

void UExtHealthComponent::ClearGameplayTags()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(ExtAbilitySystemTags::TAG_Status_Death_Dying, 0);
		AbilitySystemComponent->SetLooseGameplayTagCount(ExtAbilitySystemTags::TAG_Status_Death_Dead, 0);
	}
}

void UExtHealthComponent::StartDeath()
{
	if (DeathState != EExtDeathState::NotDead) return;
	DeathState = EExtDeathState::DeathStarted;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(ExtAbilitySystemTags::TAG_Status_Death_Dying, 1);
	}

	AActor* Owner = GetOwner();
	check(Owner);
	OnDeathStarted.Broadcast(Owner);
	Owner->ForceNetUpdate();
}

void UExtHealthComponent::FinishDeath()
{
	if (DeathState != EExtDeathState::DeathStarted) return;
	DeathState = EExtDeathState::DeathFinished;

	if (AbilitySystemComponent)
	{
		// 与 Lyra 一致，结束时保留 Dying，注销 ASC 时统一清理两个标签。
		AbilitySystemComponent->SetLooseGameplayTagCount(ExtAbilitySystemTags::TAG_Status_Death_Dead, 1);
	}

	AActor* Owner = GetOwner();
	check(Owner);
	OnDeathFinished.Broadcast(Owner);
	Owner->ForceNetUpdate();
}

void UExtHealthComponent::DamageSelfDestruct(bool bFellOutOfWorld)
{
	// 项目额外约束：蓝图和 C++ 调用都只允许服务器应用自毁伤害。
	if (!GetOwner()->HasAuthority() || IsDeadOrDying() || !AbilitySystemComponent) return;

	const UExtAbilitySystemSettings* Settings = GetDefault<UExtAbilitySystemSettings>();
	if (!Settings->DamageGameplayEffect_SetByCaller)
	{
		UE_LOG(LogAbilityExtension, Error, TEXT("DamageSelfDestruct on %s requires a damage GE in Ability Extension Settings."), *GetNameSafe(GetOwner()));
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
		Settings->DamageGameplayEffect_SetByCaller,
		1.0f,
		AbilitySystemComponent->MakeEffectContext());

	if (!SpecHandle.IsValid())
	{
		UE_LOG(LogAbilityExtension, Error, TEXT("DamageSelfDestruct on %s failed to create the damage spec."), *GetNameSafe(GetOwner()));
		return;
	}

	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();

	Spec->AddDynamicAssetTag(ExtAbilitySystemTags::TAG_Gameplay_DamageSelfDestruct);
	if (bFellOutOfWorld) Spec->AddDynamicAssetTag(ExtAbilitySystemTags::TAG_Gameplay_FellOutOfWorld);

	Spec->SetSetByCallerMagnitude(ExtAbilitySystemTags::TAG_DataTag_SetByCaller_Damage, GetMaxHealth());
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec);
}

void UExtHealthComponent::OnRep_DeathState(EExtDeathState OldDeathState)
{
	const EExtDeathState NewDeathState = DeathState;
	DeathState = OldDeathState;

	// 保留已预测的进度，不随滞后的服务器状态倒退。
	if (OldDeathState > NewDeathState)
	{
		UE_LOG(LogAbilityExtension, Warning, TEXT("HealthComponent on %s predicted past server death state [%d] -> [%d]."), *GetNameSafe(GetOwner()), static_cast<uint8>(OldDeathState), static_cast<uint8>(NewDeathState));
		return;
	}

	if (OldDeathState == EExtDeathState::NotDead)
	{
		if (NewDeathState == EExtDeathState::DeathStarted)
		{
			StartDeath();
		}
		else if (NewDeathState == EExtDeathState::DeathFinished)
		{
			// 网络可能直接复制最终状态，仍按顺序补发开始与结束通知。
			StartDeath();
			FinishDeath();
		}
		else
		{
			UE_LOG(LogAbilityExtension, Error, TEXT("HealthComponent on %s received invalid death transition [%d] -> [%d]."), *GetNameSafe(GetOwner()), static_cast<uint8>(OldDeathState), static_cast<uint8>(NewDeathState));
		}
	}
	else if (OldDeathState == EExtDeathState::DeathStarted)
	{
		if (NewDeathState == EExtDeathState::DeathFinished)
		{
			FinishDeath();
		}
		else
		{
			UE_LOG(LogAbilityExtension, Error, TEXT("HealthComponent on %s received invalid death transition [%d] -> [%d]."), *GetNameSafe(GetOwner()), static_cast<uint8>(OldDeathState), static_cast<uint8>(NewDeathState));
		}
	}

	ensureMsgf(DeathState == NewDeathState, TEXT("HealthComponent on %s failed death transition [%d] -> [%d]."), *GetNameSafe(GetOwner()), static_cast<uint8>(OldDeathState), static_cast<uint8>(NewDeathState));
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

void UExtHealthComponent::HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
#if WITH_SERVER_CODE
	if (!AbilitySystemComponent || !DamageEffectSpec || !GetOwner()->HasAuthority()) return;

	// 在 GE 结算后发送，保留 MetaDamage 转换前的效果来源和原始幅值。
	FGameplayEventData Payload;
	Payload.EventTag = ExtAbilitySystemTags::TAG_GameplayEvent_Death;
	Payload.Instigator = DamageInstigator;
	Payload.Target = AbilitySystemComponent->GetAvatarActor();
	Payload.OptionalObject = DamageEffectSpec->Def;
	Payload.ContextHandle = DamageEffectSpec->GetEffectContext();
	Payload.InstigatorTags = *DamageEffectSpec->CapturedSourceTags.GetAggregatedTags();
	Payload.TargetTags = *DamageEffectSpec->CapturedTargetTags.GetAggregatedTags();
	Payload.EventMagnitude = DamageMagnitude;

	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent, true);
	AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
#endif
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
