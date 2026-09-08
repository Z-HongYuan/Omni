// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Abilities/GameplayAbility.h"
#include "Data/CustomAbilitySet.h"
#include "GameFeatures/GameFeatureAction_WorldActionBase.h"

#include "GameFeatureAction_AddAbilities.generated.h"

#define UE_API OMNIGAME_API

struct FWorldContext;
class UInputAction;
class UAttributeSet;
class UDataTable;
struct FComponentRequestHandle;

USTRUCT(BlueprintType)
struct FAbilityGrant
{
	GENERATED_BODY()

	// 要授予的能力类型
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AssetBundles="Client,Server"))
	TSoftClassPtr<UGameplayAbility> AbilityType;
};

USTRUCT(BlueprintType)
struct FAttributeSetGrant
{
	GENERATED_BODY()

	// 要授予的属性集类型
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AssetBundles="Client,Server"))
	TSoftClassPtr<UAttributeSet> AttributeSetType;

	// 用于初始化属性的数据表，可选（可留空）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AssetBundles="Client,Server"))
	TSoftObjectPtr<UDataTable> InitializationData;
};

USTRUCT()
struct FGameFeatureAbilitiesEntry
{
	GENERATED_BODY()

	// 要添加扩展的目标 Actor 类
	UPROPERTY(EditAnywhere, Category="Abilities")
	TSoftClassPtr<AActor> ActorClass;

	// 给该类 Actor 授予的能力列表
	UPROPERTY(EditAnywhere, Category="Abilities")
	TArray<FAbilityGrant> GrantedAbilities;

	// 给该类 Actor 授予的属性集列表
	UPROPERTY(EditAnywhere, Category="Attributes")
	TArray<FAttributeSetGrant> GrantedAttributes;

	// 给该类 Actor 授予的能力集列表
	UPROPERTY(EditAnywhere, Category="Attributes", meta=(AssetBundles="Client,Server"))
	TArray<TSoftObjectPtr<const UCustomAbilitySet>> GrantedAbilitySets;
};

/**
 * 给指定类 Actor 授予能力（含属性集）的 GameFeatureAction
 *
 * 职责：
 * - 功能激活时按 AbilitiesList 向 GameFrameworkComponentManager 注册扩展处理器，
 *   目标 Actor 就绪（或 PlayerState 广播 NAME_GiveCustomAbilityReady）时授予能力/属性集/能力集
 * - 功能反激活时按记录逐项回收（SetRemoveAbilityOnEnd / RemoveSpawnedAttribute / GrantedHandles.TakeFromAbilitySystem）
 *
 * 注意：
 * - 只在服务器（HasAuthority）执行授予；客户端通过复制看到结果
 * - 目标 Actor 上没有 ASC 时，会通过组件请求系统给目标类补挂一个 UAbilitySystemComponent
 *
 * 与 Lyra 的差异:
 * 1. FLyraAbilityGrant / FLyraAttributeSetGrant 去掉 Lyra 前缀（FAbilityGrant / FAttributeSetGrant）；
 *    Lyra 里注释掉的 InputAction 字段未保留
 * 2. 就绪事件用 AExperiencePlayerState::NAME_GiveCustomAbilityReady 对应 Lyra 的 NAME_LyraAbilityReady
 * 3. 能力集/ASC 用本项目的 UCustomAbilitySet / UCustomAbilitySystemComponent
 *    （GiveToAbilitySystem 多一个 SourceObject 默认参，调用处写法不变）
 */
UCLASS(MinimalAPI, meta = (DisplayName = "添加能力"))
class UGameFeatureAction_AddAbilities final : public UGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

public:
	//~UGameFeatureAction interface
	UE_API virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	UE_API virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	//~End of UGameFeatureAction interface

	//~UObject interface
#if WITH_EDITOR
	UE_API virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~End of UObject interface

	/** 每条记录描述"给哪个 Actor 类授予哪些东西" */
	UPROPERTY(EditAnywhere, Category="Abilities", meta=(TitleProperty="ActorClass", ShowOnlyInnerProperties))
	TArray<FGameFeatureAbilitiesEntry> AbilitiesList;

private:
	// 单个 Actor 被授予内容的记录，反激活时据此回收
	struct FActorExtensions
	{
		TArray<FGameplayAbilitySpecHandle> Abilities;
		TArray<UAttributeSet*> Attributes;
		TArray<FCustomAbilitySet_GrantedHandles> AbilitySetHandles;
	};

	// 每个 GameFeatureStateChangeContext（约等于每个 GameInstance）的活跃扩展数据
	struct FPerContextData
	{
		TMap<AActor*, FActorExtensions> ActiveExtensions;
		TArray<TSharedPtr<FComponentRequestHandle>> ComponentRequests;
	};

	TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;

	//~UGameFeatureAction_WorldActionBase interface
	UE_API virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;
	//~End of UGameFeatureAction_WorldActionBase interface

	UE_API void Reset(FPerContextData& ActiveData);
	UE_API void HandleActorExtension(AActor* Actor, FName EventName, int32 EntryIndex, FGameFeatureStateChangeContext ChangeContext);
	UE_API void AddActorAbilities(AActor* Actor, const FGameFeatureAbilitiesEntry& AbilitiesEntry, FPerContextData& ActiveData);
	UE_API void RemoveActorAbilities(AActor* Actor, FPerContextData& ActiveData);

	template<class ComponentType>
	ComponentType* FindOrAddComponentForActor(AActor* Actor, const FGameFeatureAbilitiesEntry& AbilitiesEntry, FPerContextData& ActiveData)
	{
		return Cast<ComponentType>(FindOrAddComponentForActor(ComponentType::StaticClass(), Actor, AbilitiesEntry, ActiveData));
	}
	UE_API UActorComponent* FindOrAddComponentForActor(UClass* ComponentType, AActor* Actor, const FGameFeatureAbilitiesEntry& AbilitiesEntry, FPerContextData& ActiveData);
};

#undef UE_API
