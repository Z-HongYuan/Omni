// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Abilities/GameplayAbility.h"
#include "InteractionOption.generated.h"

class UAbilitySystemComponent;
class UGameplayAbility;
class IInteractableTargetInterface;

/*
 * 交互选项
 */
USTRUCT(BlueprintType, MinimalAPI)
struct FInteractionOption
{
	GENERATED_BODY()

public:
	/** 可互动目标的接口 */
	UPROPERTY(BlueprintReadWrite)
	TScriptInterface<IInteractableTargetInterface> InteractableTarget;

	/** 互动的简单文本选项 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Text;

	/** 互动的简单潜台词 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SubText;

	//
	// 方法1: 给角色一个技能,当交互时，赋予角色技能

	/** 当角色靠近可互动物体时，赋予角色能力. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> InteractionAbilityToGrant;

	//
	// 方法2: 可交互目标自己拥有技能组件,使之激活

	/** 目标上的能力系统可用于TargetInteractionHandle并发送事件（如有需要） */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UAbilitySystemComponent> TargetAbilitySystem = nullptr;

	/** 这个选项需要在物体上激活的能力专精 */
	UPROPERTY(BlueprintReadOnly)
	FGameplayAbilitySpecHandle TargetInteractionAbilityHandle;

	//
	// UI

	/** 这类互动的控件*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<UUserWidget> InteractionWidgetClass;

	//
	// 重载运算符

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
