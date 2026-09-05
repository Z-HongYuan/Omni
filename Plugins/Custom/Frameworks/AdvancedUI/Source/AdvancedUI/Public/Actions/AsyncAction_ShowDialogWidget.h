// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncAction_ShowDialogWidget.generated.h"

#define UE_API ADVANCEDUI_API

class ULocalPlayer;
class UDialogWidgetDescriptorBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDialogMessagingResultTaskDelegate, FGameplayTag, Result);

/**
 * 触发对话框,并且获取结果 (通过MessagingManager触发)
 */
UCLASS(MinimalAPI, BlueprintType)
class UAsyncAction_ShowDialogWidget : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/*
	 * 使用开发者设置中的 DialogWidget 映射 推送对应 Widget 到 UI 中 (固定是Modal层)
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta = (BlueprintInternalUseOnly = "true", WorldContext = "InWorldContextObject"), Category=AdvancedUI)
	static UAsyncAction_ShowDialogWidget* ShowDialogYesNo(
		UObject* InWorldContextObject, FGameplayTag InWidgetTag, FText Title, FText Message
	);
	/*
	 * 使用开发者设置中的 DialogWidget 映射 推送对应 Widget 到 UI 中 (固定是Modal层)
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta = (BlueprintInternalUseOnly = "true", WorldContext = "InWorldContextObject"), Category=AdvancedUI)
	static UAsyncAction_ShowDialogWidget* ShowDialogOkCancel(
		UObject* InWorldContextObject, FGameplayTag InWidgetTag, FText Title, FText Message
	);
	/*
	 * 使用开发者设置中的 DialogWidget 映射 推送对应 Widget 到 UI 中 (固定是Modal层)
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta = (BlueprintInternalUseOnly = "true", WorldContext = "InWorldContextObject"), Category=AdvancedUI)
	static UAsyncAction_ShowDialogWidget* ShowDialogCustom(
		UObject* InWorldContextObject, FGameplayTag InWidgetTag, UDialogWidgetDescriptorBase* Descriptor
	);

	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable)
	FDialogMessagingResultTaskDelegate OnResult;

private:
	void HandleConfirmationResult(FGameplayTag ConfirmationResult);

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
