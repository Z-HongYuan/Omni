// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "DialogWidgetDescriptorBase.generated.h"

#define UE_API GAMEUI_API

/** 对话框按钮的结果标签及可选显示文本。 */
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

/** 描述对话框标题、正文和按钮选项。 */
UCLASS(MinimalAPI)
class UDialogWidgetDescriptorBase : public UObject
{
	GENERATED_BODY()

public:
	static UE_API UDialogWidgetDescriptorBase* CreateConfirmationOk(const FText& Title, const FText& Body);
	static UE_API UDialogWidgetDescriptorBase* CreateConfirmationOkCancel(const FText& Title, const FText& Body);
	static UE_API UDialogWidgetDescriptorBase* CreateConfirmationYesNo(const FText& Title, const FText& Body);
	static UE_API UDialogWidgetDescriptorBase* CreateConfirmationYesNoCancel(const FText& Title, const FText& Body);

	/** 对话框标题。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	/** 对话框正文。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Body;

	/** 对话框的按钮选项及对应结果。 */
	UPROPERTY(BlueprintReadWrite)
	TArray<FConfirmationDialogAction> ButtonActions;
};

#undef UE_API
