// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncAction_ShowDialogWidget.generated.h"

#define UE_API GAMEUI_API

class ULocalPlayer;
class UDialogWidgetDescriptorBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDialogMessagingResultTaskDelegate, FGameplayTag, Result);

/** 通过 MessagingManager 显示对话框，并将选择结果广播给蓝图。 */
UCLASS(MinimalAPI, BlueprintType)
class UAsyncAction_ShowDialogWidget : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/** 在 Modal 层显示是/否对话框，控件类由设置中的标签映射决定。 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta = (BlueprintInternalUseOnly = "true", WorldContext = "InWorldContextObject"), Category=GameUI)
	static UAsyncAction_ShowDialogWidget* ShowDialogYesNo(
		UObject* InWorldContextObject, FGameplayTag InWidgetTag, FText Title, FText Message
	);
	/** 在 Modal 层显示确定/取消对话框，控件类由设置中的标签映射决定。 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta = (BlueprintInternalUseOnly = "true", WorldContext = "InWorldContextObject"), Category=GameUI)
	static UAsyncAction_ShowDialogWidget* ShowDialogOkCancel(
		UObject* InWorldContextObject, FGameplayTag InWidgetTag, FText Title, FText Message
	);
	/** 在 Modal 层显示自定义描述的对话框，控件类由设置中的标签映射决定。 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta = (BlueprintInternalUseOnly = "true", WorldContext = "InWorldContextObject"), Category=GameUI)
	static UAsyncAction_ShowDialogWidget* ShowDialogCustom(
		UObject* InWorldContextObject, FGameplayTag InWidgetTag, UDialogWidgetDescriptorBase* Descriptor
	);

	virtual void Activate() override;

	/** 返回对话结果；无法展示时返回空标签，结束调用方等待。 */
	UPROPERTY(BlueprintAssignable)
	FDialogMessagingResultTaskDelegate OnResult;

private:
	void HandleConfirmationResult(FGameplayTag ConfirmationResult);
	bool bActivated = false;
	bool bFinished = false;

	UPROPERTY(Transient)
	TObjectPtr<UObject> WorldContextObject;

	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> TargetLocalPlayer;

	UPROPERTY(Transient)
	FGameplayTag WidgetTag;

	UPROPERTY(Transient)
	TObjectPtr<UDialogWidgetDescriptorBase> Descriptor;
};

#undef UE_API
