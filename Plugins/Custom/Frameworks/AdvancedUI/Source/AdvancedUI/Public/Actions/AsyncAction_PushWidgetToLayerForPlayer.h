// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_PushWidgetToLayerForPlayer.generated.h"

#define UE_API ADVANCEDUI_API

struct FStreamableHandle;
class UCommonActivatableWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPushWidgetToLayerForPlayerAsyncDelegate, UCommonActivatableWidget*, UserWidget);

/**
 * 异步推送控件到容器内
 */
UCLASS(MinimalAPI, BlueprintType)
class UAsyncAction_PushWidgetToLayerForPlayer : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta=(WorldContext = "WorldContextObject", BlueprintInternalUseOnly="true"), Category=AdvancedUI)
	static UE_API UAsyncAction_PushWidgetToLayerForPlayer* PushWidgetToLayerForPlayer(APlayerController* InOwningPlayer,
	                                                                                  UPARAM(meta = (AllowAbstract=false)) TSoftClassPtr<UCommonActivatableWidget> InWidgetClass,
	                                                                                  UPARAM(meta = (Categories = "AdvancedUI.UIStack")) FGameplayTag InLayerName,
	                                                                                  bool bSuspendInputUntilComplete = true);

	UE_API virtual void Activate() override;

	UE_API virtual void Cancel() override;

	UPROPERTY(BlueprintAssignable)
	FPushWidgetToLayerForPlayerAsyncDelegate BeforePush;

	UPROPERTY(BlueprintAssignable)
	FPushWidgetToLayerForPlayerAsyncDelegate AfterPush;

private:
	FGameplayTag LayerName;
	bool bSuspendInputUntilComplete = false;
	TWeakObjectPtr<APlayerController> OwningPlayerPtr;
	TSoftClassPtr<UCommonActivatableWidget> WidgetClass;
	TSharedPtr<FStreamableHandle> StreamingHandle;
};

#undef UE_API
