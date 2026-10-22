// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_PushWidgetToLayerForPlayer.generated.h"

#define UE_API GAMEUI_API

struct FStreamableHandle;
class UCommonActivatableWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPushWidgetToLayerForPlayerAsyncDelegate, UCommonActivatableWidget*, UserWidget);

/** 异步加载控件类，并将实例推入所属本地玩家的指定 UI 层。 */
UCLASS(MinimalAPI, BlueprintType)
class UAsyncAction_PushWidgetToLayerForPlayer : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	/** 可选择在加载期间暂停输入；失败或取消时不广播 AfterPush。 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta=(BlueprintInternalUseOnly="true"), Category=GameUI)
	static UE_API UAsyncAction_PushWidgetToLayerForPlayer* PushWidgetToLayerForPlayer(APlayerController* InOwningPlayer,
	                                                                                  UPARAM(meta = (AllowAbstract=false)) TSoftClassPtr<UCommonActivatableWidget> InWidgetClass,
	                                                                                  UPARAM(meta = (Categories = "GameUI.UIStack")) FGameplayTag InLayerName,
	                                                                                  bool bSuspendInputUntilComplete = true);

	UE_API virtual void Activate() override;

	UE_API virtual void Cancel() override;

	/** 控件加入堆栈前的初始化通知；回调中可以取消本次推送。 */
	UPROPERTY(BlueprintAssignable)
	FPushWidgetToLayerForPlayerAsyncDelegate BeforePush;

	/** 控件成功加入堆栈后的完成通知。 */
	UPROPERTY(BlueprintAssignable)
	FPushWidgetToLayerForPlayerAsyncDelegate AfterPush;

private:
	bool bActivated = false;
	bool bFinished = false;
	FGameplayTag LayerName;
	bool bSuspendInputUntilComplete = false;
	TWeakObjectPtr<APlayerController> OwningPlayerPtr;
	TSoftClassPtr<UCommonActivatableWidget> WidgetClass;
	TSharedPtr<FStreamableHandle> StreamingHandle;
};

#undef UE_API
