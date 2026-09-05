// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "DialogWidgetDescriptorBase.generated.h"

#define UE_API ADVANCEDUI_API

/* 对话框的按钮所代表的结果等参数 */
USTRUCT(BlueprintType)
struct FConfirmationDialogAction
{
	GENERATED_BODY()

public:
	/** 必选：提供对话选项。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag ActionResult = FGameplayTag::EmptyTag;

	/** 可选：用于代替与结果关联的操作名称的显示文本。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText OptionalDisplayText;

	bool operator==(const FConfirmationDialogAction& Other) const
	{
		return ActionResult == Other.ActionResult && OptionalDisplayText.EqualTo(Other.OptionalDisplayText);
	}
};

/**
 * 对话系统使用的数据基类,带有最基础的对话框信息,可以自定义按钮选项
 */
UCLASS(MinimalAPI)
class UDialogWidgetDescriptorBase : public UObject
{
	GENERATED_BODY()

public:
	static UE_API UDialogWidgetDescriptorBase* CreateConfirmationOk(const FText& Title, const FText& Body);
	static UE_API UDialogWidgetDescriptorBase* CreateConfirmationOkCancel(const FText& Title, const FText& Body);
	static UE_API UDialogWidgetDescriptorBase* CreateConfirmationYesNo(const FText& Title, const FText& Body);
	static UE_API UDialogWidgetDescriptorBase* CreateConfirmationYesNoCancel(const FText& Title, const FText& Body);

	/** 要显示的消息头 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	/** 要显示的消息正文 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Body;

	/** 确认按钮的输入动作。 */
	UPROPERTY(BlueprintReadWrite)
	TArray<FConfirmationDialogAction> ButtonActions;
};

#undef UE_API
