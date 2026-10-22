// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Abilities/GameplayAbility.h"
#include "InteractionOption.generated.h"

class UAbilitySystemComponent;
class UGameplayAbility;
class IInteractableTargetInterface;

/** 描述交互目标、能力触发方式和界面展示内容。 */
USTRUCT(BlueprintType, MinimalAPI)
struct FInteractionOption
{
	GENERATED_BODY()

public:
	/** 提供该选项的交互目标。 */
	UPROPERTY(BlueprintReadWrite)
	TScriptInterface<IInteractableTargetInterface> InteractableTarget;

	/** 交互选项的主文本。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Text;

	/** 交互选项的补充文本。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SubText;

	// 方式一：由附近扫描任务给交互发起方授予能力。

	/** 附近扫描任务需要授予交互发起方的能力类型。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> InteractionAbilityToGrant;

	// 方式二：使用指定能力系统中已有的能力。

	/** 承载待触发能力的能力系统，与 TargetInteractionAbilityHandle 配合使用。 */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UAbilitySystemComponent> TargetAbilitySystem = nullptr;

	/** 在 TargetAbilitySystem 中触发交互所用的能力句柄。 */
	UPROPERTY(BlueprintReadOnly)
	FGameplayAbilitySpecHandle TargetInteractionAbilityHandle;

	// 界面展示。

	/** 该交互选项使用的指示器控件类。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<UUserWidget> InteractionWidgetClass;

	// 选项比较与排序。

	FORCEINLINE bool operator==(const FInteractionOption& Other) const
	{
		return InteractableTarget == Other.InteractableTarget &&
			InteractionAbilityToGrant == Other.InteractionAbilityToGrant &&
			TargetAbilitySystem == Other.TargetAbilitySystem &&
			TargetInteractionAbilityHandle == Other.TargetInteractionAbilityHandle &&
			InteractionWidgetClass == Other.InteractionWidgetClass &&
			Text.IdenticalTo(Other.Text) &&
			SubText.IdenticalTo(Other.SubText);
	}

	FORCEINLINE bool operator!=(const FInteractionOption& Other) const
	{
		return !operator==(Other);
	}

	FORCEINLINE bool operator<(const FInteractionOption& Other) const
	{
		return InteractableTarget.GetInterface() < Other.InteractableTarget.GetInterface();
	}
};
