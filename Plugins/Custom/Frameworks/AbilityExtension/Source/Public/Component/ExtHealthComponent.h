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
 * ASC 初始化完成后，由外部调用本组件，查找已有的健康属性集并绑定 ASC 属性变更委托。
 * 对外提供生命值查询、生命变化与归零事件；注销时解除绑定，OnOutOfHealth 仅在服务器广播。
 * 沿用 Lyra 的三阶段死亡状态、复制通知及 Dying/Dead 标签；开始和结束由外部死亡流程驱动。
 * GE 结算归零时向 ASC 发送 GameplayEvent.Death，由已授予的死亡技能驱动死亡流程。
 * 自毁沿用 Lyra 的伤害 GE 路径；项目适配为读取 DeveloperSettings 的 GE，使用原生伤害数值 Tag，并显式限制服务器执行。
 * 对照 Lyra：绑定时不回满生命，普通属性通知仍使用 ASC 委托；尚未接入淘汰消息。
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

	// 出生时显式调用一次；服务器恢复为当前最大生命，绑定和重绑本身不回血。
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

	// 死亡流程调用；客户端 OnRep 也通过这两个入口补齐状态和通知。
	UE_API virtual void StartDeath();
	UE_API virtual void FinishDeath();

	// 向自身应用最大生命值的伤害，死亡表现和结束仍由已授予的死亡技能负责。
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
	// 属于当前 Pawn 的出生过程，注销 ASC 不重置，避免重绑变成免费治疗。
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
