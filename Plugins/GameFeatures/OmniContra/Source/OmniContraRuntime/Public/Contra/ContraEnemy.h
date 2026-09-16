// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "AbilitySystemInterface.h"
#include "GameFramework/Actor.h"
#include "ContraEnemy.generated.h"

class UExtAbilitySystemComponent;
class UExtHealthSet;
class UExtHealthComponent;
class UExtDeathComponent;
class UStaticMeshComponent;

/** 首轮固定射手：自身持有 ASC，周期检查最近的活玩家及遮挡。用蓝图配置占位外观。 */
UCLASS()
class OMNICONTRARUNTIME_API AContraEnemy : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AContraEnemy();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type Reason) override;
	UFUNCTION(BlueprintPure, Category = "Contra")
	bool IsDead() const;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Contra")
	TObjectPtr<UStaticMeshComponent> Body;
	UPROPERTY(EditAnywhere, Category = "Contra", meta = (ClampMin = "0.2"))
	float AttackInterval = 1.5f;
	UPROPERTY(EditAnywhere, Category = "Contra", meta = (ClampMin = "1"))
	float AttackDamage = 20.0f;
	UPROPERTY(EditAnywhere, Category = "Contra", meta = (ClampMin = "1"))
	float AttackRange = 1000.0f;

private:
	void Attack();
	UFUNCTION()
	void OutOfHealth(UExtHealthComponent* Component, float OldValue, float NewValue, AActor* InstigatorActor);
	UFUNCTION()
	void DeathStarted(AActor* OwnerActor);
	UFUNCTION(NetMulticast, Unreliable)
	void ShowShot(FVector_NetQuantize Target);
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UExtAbilitySystemComponent> AbilitySystem;
	UPROPERTY()
	TObjectPtr<UExtHealthSet> HealthSet;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UExtHealthComponent> Health;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UExtDeathComponent> Death;
	FTimerHandle AttackTimer;
};
