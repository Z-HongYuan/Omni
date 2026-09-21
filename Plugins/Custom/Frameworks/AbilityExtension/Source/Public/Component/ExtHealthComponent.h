// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/GameFrameworkComponent.h"
#include "ExtHealthComponent.generated.h"

#define UE_API ABILITYEXTENSION_API

class UExtAbilitySystemComponent;
class UExtHealthComponent;
class UExtHealthSet;
struct FOnAttributeChangeData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FExtHealthAttributeChanged, UExtHealthComponent*, HealthComponent, float, OldValue, float, NewValue, AActor*, Instigator);

/**
 * ASC 初始化完成后，由外部调用本组件，查找已有的健康属性集并绑定 ASC 属性变更委托。
 * 对外提供生命值查询、生命变化与归零事件；注销时解除绑定，OnOutOfHealth 仅在服务器广播。
 * 对照 Lyra：当前只负责属性访问和事件转发，绑定时不回满生命；死亡状态、死亡能力事件及重生流程尚未接入。
 */
UCLASS(MinimalAPI, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UExtHealthComponent : public UGameFrameworkComponent
{
	GENERATED_BODY()

public:
	UE_API UExtHealthComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintPure, Category = "AbilityExtension|Health")
	static UE_API UExtHealthComponent* FindHealthComponent(const AActor* Actor);

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

	UPROPERTY(BlueprintAssignable, Category = "AbilityExtension|Health")
	FExtHealthAttributeChanged OnHealthChanged;
	UPROPERTY(BlueprintAssignable, Category = "AbilityExtension|Health")
	FExtHealthAttributeChanged OnMaxHealthChanged;
	UPROPERTY(BlueprintAssignable, Category = "AbilityExtension|Health")
	FExtHealthAttributeChanged OnOutOfHealth;

	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	UE_API virtual void OnUnregister() override;

private:
	// 属于当前 Pawn 的出生过程，注销 ASC 不重置，避免重绑变成免费治疗。
	bool bHasInitializedSpawnHealth = false;

	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);

	UPROPERTY(Transient)
	TObjectPtr<UExtAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY(Transient)
	TObjectPtr<const UExtHealthSet> HealthSet;

	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;
};

#undef UE_API
