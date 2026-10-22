// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/GameFrameworkComponent.h"
#include "ExtHealthComponent.generated.h"

#define UE_API ABILITYEXTENSION_API

class UExtAbilitySystemComponent;
class UExtHealthComponent;
class UExtHealthSet;
struct FOnAttributeChangeData;
struct FGameplayEffectSpec;

UENUM(BlueprintType)
enum class EExtDeathState : uint8
{
	NotDead,
	DeathStarted,
	DeathFinished
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FExtHealthAttributeChanged, UExtHealthComponent*, HealthComponent, float, OldValue, float, NewValue, AActor*, Instigator);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FExtHealthDeathEvent, AActor*, OwningActor);

/**
 * 在 ASC 就绪后显式绑定已有的生命属性集，提供生命查询及变化通知，解绑时清理委托。
 * 维护并复制三阶段死亡状态及 Dying/Dead 标签，开始和结束由外部死亡流程驱动。
 * 服务器在 GE 结算归零时发送 GameplayEvent.Death，自毁通过配置的伤害 GE 结算。
 * 绑定或重绑不会回满生命，出生初始化须显式调用。
 */
UCLASS(MinimalAPI, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UExtHealthComponent : public UGameFrameworkComponent
{
	GENERATED_BODY()

public:
	UE_API UExtHealthComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintPure, Category = "AbilityExtension|Health")
	static UE_API UExtHealthComponent* FindHealthComponent(const AActor* Actor) { return Actor ? Actor->FindComponentByClass<UExtHealthComponent>() : nullptr; };

	UFUNCTION(BlueprintCallable, Category = "AbilityExtension|Health")
	UE_API void InitializeWithAbilitySystem(UExtAbilitySystemComponent* InASC);
	UFUNCTION(BlueprintCallable, Category = "AbilityExtension|Health")
	UE_API void UninitializeFromAbilitySystem();

	/** 出生时显式调用一次；仅服务器恢复为当前最大生命，绑定和重绑本身不回血。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "AbilityExtension|Health")
	UE_API bool InitializeHealthForSpawn();

	UFUNCTION(BlueprintPure, Category = "AbilityExtension|Health")
	UE_API float GetHealth() const;
	UFUNCTION(BlueprintPure, Category = "AbilityExtension|Health")
	UE_API float GetMaxHealth() const;
	UFUNCTION(BlueprintPure, Category = "AbilityExtension|Health")
	UE_API float GetHealthNormalized() const;

	UFUNCTION(BlueprintPure, Category = "AbilityExtension|Health")
	EExtDeathState GetDeathState() const { return DeathState; }

	UFUNCTION(BlueprintPure, Category = "AbilityExtension|Health")
	bool IsDeadOrDying() const { return DeathState != EExtDeathState::NotDead; }

	/** 从存活进入死亡开始状态；客户端复制通知也通过此入口补齐状态。 */
	UE_API virtual void StartDeath();
	/** 从死亡开始进入死亡完成状态；客户端可在复制最终状态时补调。 */
	UE_API virtual void FinishDeath();

	/** 仅服务器向自身应用最大生命值的伤害，死亡表现和结束由已授予的死亡能力负责。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "AbilityExtension|Health")
	UE_API virtual void DamageSelfDestruct(bool bFellOutOfWorld = false);

	UPROPERTY(BlueprintAssignable, Category = "AbilityExtension|Health")
	FExtHealthAttributeChanged OnHealthChanged;
	UPROPERTY(BlueprintAssignable, Category = "AbilityExtension|Health")
	FExtHealthAttributeChanged OnMaxHealthChanged;
	UPROPERTY(BlueprintAssignable, Category = "AbilityExtension|Health")
	FExtHealthAttributeChanged OnOutOfHealth;

	UPROPERTY(BlueprintAssignable, Category = "AbilityExtension|Health")
	FExtHealthDeathEvent OnDeathStarted;
	UPROPERTY(BlueprintAssignable, Category = "AbilityExtension|Health")
	FExtHealthDeathEvent OnDeathFinished;

protected:
	UE_API virtual void OnUnregister() override;

	UFUNCTION()
	UE_API virtual void OnRep_DeathState(EExtDeathState OldDeathState);

	UPROPERTY(ReplicatedUsing = OnRep_DeathState)
	EExtDeathState DeathState = EExtDeathState::NotDead;

private:
	/** 当前 Pawn 是否已初始化出生生命；解绑 ASC 时不重置，避免重绑触发治疗。 */
	bool bHasInitializedSpawnHealth = false;

	void ClearGameplayTags();

	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);
	void HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);

	UPROPERTY(Transient)
	TObjectPtr<UExtAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY(Transient)
	TObjectPtr<const UExtHealthSet> HealthSet;

	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;
	FDelegateHandle OutOfHealthHandle;
};

#undef UE_API
