// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpecHandle.h"
#include "ExtGlobalAbilityManager.generated.h"

#define UE_API ABILITYEXTENSION_API

class UGameplayEffect;
class UGameplayAbility;
class UExtAbilitySystemComponent;

/** 记录各 ASC 由此管理器授予的能力句柄。 */
USTRUCT()
struct FGlobalAppliedAbilityList
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<TObjectPtr<UExtAbilitySystemComponent>, FGameplayAbilitySpecHandle> Handles;

	void AddToASC(TSubclassOf<UGameplayAbility> Ability, UExtAbilitySystemComponent* ASC);
	void RemoveFromASC(UExtAbilitySystemComponent* ASC);
	void RemoveFromAll();
};

/** 记录各 ASC 由此管理器应用的活跃效果句柄。 */
USTRUCT()
struct FGlobalAppliedEffectList
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<TObjectPtr<UExtAbilitySystemComponent>, FActiveGameplayEffectHandle> Handles;

	void AddToASC(TSubclassOf<UGameplayEffect> Effect, UExtAbilitySystemComponent* ASC);
	void RemoveFromASC(UExtAbilitySystemComponent* ASC);
	void RemoveFromAll();
};


/** 按世界维护全局能力和效果，同时应用于当前及后续注册的 ASC。 */
UCLASS(MinimalAPI)
class UExtGlobalAbilityManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UE_API UExtGlobalAbilityManager() { ; }

	/** 应在权威端调用，向已注册 ASC 授予能力，并保存配置供后续注册使用。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="AbilityExtension|GlobalAbility")
	UE_API void ApplyAbilityToAll(TSubclassOf<UGameplayAbility> Ability);

	/** 应在权威端调用，向已注册 ASC 应用效果，并保存配置供后续注册使用。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="AbilityExtension|GlobalAbility")
	UE_API void ApplyEffectToAll(TSubclassOf<UGameplayEffect> Effect);

	/** 从已注册 ASC 撤销此管理器授予的指定能力。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "AbilityExtension|GlobalAbility")
	UE_API void RemoveAbilityFromAll(TSubclassOf<UGameplayAbility> Ability);

	/** 从已注册 ASC 撤销此管理器应用的指定效果。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "AbilityExtension|GlobalAbility")
	UE_API void RemoveEffectFromAll(TSubclassOf<UGameplayEffect> Effect);

	/** 登记 ASC 并应用已有的全局能力和效果；扩展 ASC 在新 Pawn 就绪时调用。 */
	UE_API void RegisterASC(UExtAbilitySystemComponent* ASC);

	/** 注销 ASC 并撤销此管理器授予的能力和效果；扩展 ASC 在 EndPlay 时调用。 */
	UE_API void UnregisterASC(UExtAbilitySystemComponent* ASC);

private:
	/** 按能力类保存各 ASC 的授予记录。 */
	UPROPERTY()
	TMap<TSubclassOf<UGameplayAbility>, FGlobalAppliedAbilityList> AppliedAbilities;

	/** 按效果类保存各 ASC 的应用记录。 */
	UPROPERTY()
	TMap<TSubclassOf<UGameplayEffect>, FGlobalAppliedEffectList> AppliedEffects;

	/** 当前注册到本管理器的 ASC。 */
	UPROPERTY()
	TArray<TObjectPtr<UExtAbilitySystemComponent>> RegisteredASCs;
};

#undef UE_API
