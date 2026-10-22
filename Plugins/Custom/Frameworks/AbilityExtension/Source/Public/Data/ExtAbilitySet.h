// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "ExtAbilitySet.generated.h"

#define UE_API ABILITYEXTENSION_API

class UExtAbilitySystemComponent;
class UAttributeSet;
class UGameplayEffect;
class UExtGameplayAbility;

/** 能力集中的能力类、等级和输入标签配置。 */
USTRUCT(BlueprintType)
struct FExtAbilitySet_GameplayAbility
{
	GENERATED_BODY()

public:
	/** 需要授予的能力类。 */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UExtGameplayAbility> Ability;

	/** 授予的能力等级。 */
	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel = 1;

	/** 写入能力 Spec 的输入标签，供 ASC 匹配输入。 */
	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};


/** 能力集中的 GameplayEffect 类与等级配置。 */
USTRUCT(BlueprintType)
struct FExtAbilitySet_GameplayEffect
{
	GENERATED_BODY()

public:
	/** 需要应用的 GameplayEffect 类。 */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GameplayEffect;

	/** 应用效果时使用的等级。 */
	UPROPERTY(EditDefaultsOnly)
	float EffectLevel = 1.0f;
};

/** 能力集中的属性集类配置。 */
USTRUCT(BlueprintType)
struct FExtAbilitySet_AttributeSet
{
	GENERATED_BODY()

public:
	/** 需要创建并注册到 ASC 的属性集类型。 */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAttributeSet> AttributeSet;
};

/** 保存一次能力集授予产生的句柄和属性集，供后续统一撤销。 */
USTRUCT(BlueprintType)
struct FExtAbilitySet_GrantedHandles
{
	GENERATED_BODY()

public:
	UE_API void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	UE_API void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);
	UE_API void AddAttributeSet(UAttributeSet* Set);

	/** 仅在权威端撤销记录中的能力、效果和属性集，并清空记录；ASC 须与授予时一致。 */
	UE_API void TakeFromAbilitySystem(UExtAbilitySystemComponent* ExtASC);

protected:
	/** 本次授予的能力句柄。 */
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	/** 本次应用的活跃效果句柄。 */
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;

	/** 本次创建并注册到 ASC 的属性集。 */
	UPROPERTY()
	TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;
};

/** 按配置批量授予能力、效果和属性集的主数据资产。 */
UCLASS(MinimalAPI, BlueprintType, Const)
class UExtAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UE_API UExtAbilitySet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * 仅在权威端向有效 ASC 授予能力集；属性集以 ASC 所属 Actor 为 Outer。
	 * OutGrantedHandles 可为空，非空时记录本次授予结果，供后续撤销。
	 */
	UE_API void GiveToAbilitySystem(UExtAbilitySystemComponent* ExtASC, FExtAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject = nullptr) const;

protected:
	/** 授予此能力集时添加的能力配置。 */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities", meta=(TitleProperty=Ability))
	TArray<FExtAbilitySet_GameplayAbility> GrantedGameplayAbilities;

	/** 授予此能力集时应用的效果配置。 */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", meta=(TitleProperty=GameplayEffect))
	TArray<FExtAbilitySet_GameplayEffect> GrantedGameplayEffects;

	/** 授予此能力集时创建的属性集配置。 */
	UPROPERTY(EditDefaultsOnly, Category = "Attribute Sets", meta=(TitleProperty=AttributeSet))
	TArray<FExtAbilitySet_AttributeSet> GrantedAttributes;
};

#undef UE_API
