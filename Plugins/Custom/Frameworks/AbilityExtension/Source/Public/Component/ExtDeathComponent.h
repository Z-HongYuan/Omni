// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/GameFrameworkComponent.h"
#include "ExtDeathComponent.generated.h"

#define UE_API ABILITYEXTENSION_API

class UExtAbilitySystemComponent;

UENUM(BlueprintType)
enum class EExtDeathState : uint8
{
	NotDead,
	DeathStarted,
	DeathFinished
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FExtDeathEvent, AActor*, OwningActor);

/**
 * 每个 Avatar 的最小死亡状态：服务端推进，客户端按复制状态补齐开始/结束通知。
 * 外部连接健康事件及 ASC；本组件只管理死亡状态、输入阻断和自身添加的标签。
 * 不扣命、不回满生命、不销毁角色；重生使用新 Pawn，不回退旧组件的状态。
 * 对照 Lyra：保留单向状态与漏帧补发，表现、死亡能力编排和重生策略交给使用方。
 */
UCLASS(MinimalAPI, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UExtDeathComponent : public UGameFrameworkComponent
{
	GENERATED_BODY()

public:
	UE_API UExtDeathComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UE_API virtual void OnUnregister() override;

	UFUNCTION(BlueprintCallable, Category = "AbilityExtension|Death")
	UE_API void InitializeWithAbilitySystem(UExtAbilitySystemComponent* InASC);
	UFUNCTION(BlueprintCallable, Category = "AbilityExtension|Death")
	UE_API void UninitializeFromAbilitySystem();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "AbilityExtension|Death")
	UE_API void StartDeath();
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "AbilityExtension|Death")
	UE_API void FinishDeath();

	UFUNCTION(BlueprintPure, Category = "AbilityExtension|Death")
	EExtDeathState GetDeathState() const { return DeathState; }

	UFUNCTION(BlueprintPure, Category = "AbilityExtension|Death")
	bool IsDeadOrDying() const { return DeathState != EExtDeathState::NotDead; }

	UPROPERTY(BlueprintAssignable, Category = "AbilityExtension|Death")
	FExtDeathEvent OnDeathStarted;
	UPROPERTY(BlueprintAssignable, Category = "AbilityExtension|Death")
	FExtDeathEvent OnDeathFinished;

private:
	UFUNCTION()
	void OnRep_DeathState(EExtDeathState OldState);
	void NotifyDeathState();

	void ApplyDeathToAbilitySystem();

	UPROPERTY(ReplicatedUsing = OnRep_DeathState)
	EExtDeathState DeathState = EExtDeathState::NotDead;

	UPROPERTY(Transient)
	TObjectPtr<UExtAbilitySystemComponent> AbilitySystemComponent;

	bool bAppliedDeathTags = false;

	EExtDeathState NotifiedState = EExtDeathState::NotDead;

	bool bDispatchingNotifications = false;
};

#undef UE_API
