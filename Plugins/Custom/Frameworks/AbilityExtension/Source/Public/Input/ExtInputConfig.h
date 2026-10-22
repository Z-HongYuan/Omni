// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "ExtInputConfig.generated.h"

#define UE_API ABILITYEXTENSION_API

class UInputAction;

/** InputAction 与 GameplayTag 的映射项。 */
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

/** 配置原生动作和能力动作的输入标签映射。 */
UCLASS(MinimalAPI, BlueprintType, Const)
class UExtInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "AbilityExtension|Input")
	UE_API const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	UFUNCTION(BlueprintCallable, Category = "AbilityExtension|Input")
	UE_API const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	/** 原生动作映射，由调用方按标签选择并绑定回调。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FExtInputAction> NativeInputActions;

	/** 能力动作映射，由 BindAbilityActions 绑定标签回调，再交给 ASC 匹配能力。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FExtInputAction> AbilityInputActions;
};

#undef UE_API
