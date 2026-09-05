// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "CustomAbilitySet.generated.h"

#define UE_API CUSTOMABILITYSYSTEM_API

class UCustomAbilitySystemComponent;
class UAttributeSet;
class UGameplayEffect;
class UCustomGameplayAbility;

/**
 *	能力集用来赋予游戏能力的数据。
 */
USTRUCT(BlueprintType)
struct FCustomAbilitySet_GameplayAbility
{
	GENERATED_BODY()

public:
	// 游戏能力授予。
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCustomGameplayAbility> Ability;

	// 授予的能力等级。
	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel = 1;

	// 用于处理能力输入的标签。
	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};


/**
 *	能力集用于授予游戏效果的数据。
 */
USTRUCT(BlueprintType)
struct FCustomAbilitySet_GameplayEffect
{
	GENERATED_BODY()

public:
	// 游戏效果授予。
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GameplayEffect;

	// 要授予的游戏效果级别。
	UPROPERTY(EditDefaultsOnly)
	float EffectLevel = 1.0f;
};

/**
 *	能力集用于授予属性集的数据。
 */
USTRUCT(BlueprintType)
struct FCustomAbilitySet_AttributeSet
{
	GENERATED_BODY()

public:
	// 游戏效果授予。
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAttributeSet> AttributeSet;
};

/**
 *	用于存储能力集授予的句柄的数据。
 *	此处应用了Ability和InputTag之间的关系
 */
USTRUCT(BlueprintType)
struct FCustomAbilitySet_GrantedHandles
{
	GENERATED_BODY()

public:
	UE_API void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	UE_API void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);
	UE_API void AddAttributeSet(UAttributeSet* Set);

	UE_API void TakeFromAbilitySystem(UCustomAbilitySystemComponent* CustomASC);

protected:
	// 处理授予的能力。
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	// 处理授予的游戏效果。
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;

	// 指向已授予属性集的指针
	UPROPERTY()
	TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;
};

/**
 * 
 */
UCLASS(MinimalAPI, BlueprintType, Const)
class UCustomAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UE_API UCustomAbilitySet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//将能力集授予指定的能力系统组件。
	//返回的句柄以后可以用来拿走任何已授予的东西。
	UE_API void GiveToAbilitySystem(UCustomAbilitySystemComponent* CustomASC, FCustomAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject = nullptr) const;

protected:
	// 授予此能力集时授予的游戏能力。
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities", meta=(TitleProperty=Ability))
	TArray<FCustomAbilitySet_GameplayAbility> GrantedGameplayAbilities;

	// 授予此能力集时要授予的游戏效果。
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", meta=(TitleProperty=GameplayEffect))
	TArray<FCustomAbilitySet_GameplayEffect> GrantedGameplayEffects;

	// 授予此能力集时要授予的属性集。
	UPROPERTY(EditDefaultsOnly, Category = "Attribute Sets", meta=(TitleProperty=AttributeSet))
	TArray<FCustomAbilitySet_AttributeSet> GrantedAttributes;
};

#undef UE_API
