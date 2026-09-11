// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpecHandle.h"
#include "CustomGlobalAbilityManager.generated.h"

#define UE_API GAMEABILITYSYSTEM_API

class UGameplayEffect;
class UGameplayAbility;
class UCustomAbilitySystemComponent;

// 保存已添加能力  ASC:Handle
USTRUCT()
struct FGlobalAppliedAbilityList
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<TObjectPtr<UCustomAbilitySystemComponent>, FGameplayAbilitySpecHandle> Handles;

	void AddToASC(TSubclassOf<UGameplayAbility> Ability, UCustomAbilitySystemComponent* ASC);
	void RemoveFromASC(UCustomAbilitySystemComponent* ASC);
	void RemoveFromAll();
};

// 保存已添加 Effect  ASC:Handle
USTRUCT()
struct FGlobalAppliedEffectList
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<TObjectPtr<UCustomAbilitySystemComponent>, FActiveGameplayEffectHandle> Handles;

	void AddToASC(TSubclassOf<UGameplayEffect> Effect, UCustomAbilitySystemComponent* ASC);
	void RemoveFromASC(UCustomAbilitySystemComponent* ASC);
	void RemoveFromAll();
};


/**
 * Level中的全局技能/效果管理器
 * 会对所有注册到此管理器的ASC应用技能/效果(包括已有的)
 */
UCLASS(MinimalAPI)
class UCustomGlobalAbilityManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UE_API UCustomGlobalAbilityManager() { ; }

	// 应用技能到所有注册的ASC
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="CustomGlobalAbility")
	UE_API void ApplyAbilityToAll(TSubclassOf<UGameplayAbility> Ability);

	// 应用效果到所有注册的ASC
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="CustomGlobalAbility")
	UE_API void ApplyEffectToAll(TSubclassOf<UGameplayEffect> Effect);

	// 移除技能从所有注册的ASC
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CustomGlobalAbility")
	UE_API void RemoveAbilityFromAll(TSubclassOf<UGameplayAbility> Ability);

	// 移除效果从所有注册的ASC
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CustomGlobalAbility")
	UE_API void RemoveEffectFromAll(TSubclassOf<UGameplayEffect> Effect);

	// 在注册ASC，并应用所有的全局效果 (ASC内会自动处理,但是也可以手动)
	UE_API void RegisterASC(UCustomAbilitySystemComponent* ASC);

	// 将ASC移除，同时移除所有的全局效果 (ASC内会自动处理,但是也可以手动)
	UE_API void UnregisterASC(UCustomAbilitySystemComponent* ASC);

private:
	//根据技能分类的 (ASC和Handle键值对)
	UPROPERTY()
	TMap<TSubclassOf<UGameplayAbility>, FGlobalAppliedAbilityList> AppliedAbilities;

	//根据Effect分类的 (ASC和Handle键值对)
	UPROPERTY()
	TMap<TSubclassOf<UGameplayEffect>, FGlobalAppliedEffectList> AppliedEffects;

	//注册的全部ASC
	UPROPERTY()
	TArray<TObjectPtr<UCustomAbilitySystemComponent>> RegisteredASCs;
};

#undef UE_API
