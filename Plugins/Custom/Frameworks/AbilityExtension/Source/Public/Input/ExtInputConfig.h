// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "ExtInputConfig.generated.h"

#define UE_API ABILITYEXTENSION_API

class UInputAction;

/*
 * 用于 InputAction 与 GameplayTag 的映射
 */
USTRUCT(BlueprintType)
struct FExtInputAction
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UInputAction> InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

/**
 * 用于配置输入的数据资产
 */
UCLASS(MinimalAPI, BlueprintType, Const)
class UExtInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "AbilityExtension|Input")
	UE_API const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	UFUNCTION(BlueprintCallable, Category = "AbilityExtension|Input")
	UE_API const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	// 所有者使用的输入操作列表。这些输入操作会映射到游戏标签，必须手动绑定。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FExtInputAction> NativeInputActions;

	// 所有者使用的输入操作列表。这些输入动作被映射到游戏标签上，并自动绑定到输入标签匹配的能力上。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FExtInputAction> AbilityInputActions;
};

#undef UE_API
