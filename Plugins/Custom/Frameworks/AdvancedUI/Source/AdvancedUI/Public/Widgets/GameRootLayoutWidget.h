// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "GameplayTagContainer.h"
#include "Engine/AssetManager.h"
#include "HelperFunctions/UIHelperFunctions.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "GameRootLayoutWidget.generated.h"

#define UE_API ADVANCEDUI_API

// 控件的推送状态
enum class EAsyncWidgetPushState : uint8
{
	Canceled,
	Initialize,
	AfterPush
};

/**
 * 作为每个本地玩家拥有的根控件
 */
UCLASS(MinimalAPI, Abstract, meta = (DisableNativeTick))
class UGameRootLayoutWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	// 从世界上下文获取主要玩家的根控件
	static UE_API UGameRootLayoutWidget* GetRootLayoutWidgetForPrimaryPlayer(const UObject* WorldContextObject);
	// 从 PC 获取 根控件
	static UE_API UGameRootLayoutWidget* GetRootLayoutWidget(APlayerController* PlayerController);
	// 从 本地玩家 获取 根控件
	static UE_API UGameRootLayoutWidget* GetRootLayoutWidget(ULocalPlayer* LocalPlayer);

	/** 休眠的根布局会被折叠，只响应拥有玩家记录的持久动作 */
	UE_API void SetIsDormant(bool InDormant);
	bool IsDormant() const { return bIsDormant; }
	UE_API virtual void OnIsDormantChanged();


	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	TSharedPtr<FStreamableHandle> PushWidgetToLayerStackAsync(FGameplayTag LayerName, bool bSuspendInputUntilComplete, TSoftClassPtr<UCommonActivatableWidget> ActivatableWidgetClass)
	{
		return PushWidgetToLayerStackAsync<ActivatableWidgetT>(LayerName, bSuspendInputUntilComplete, ActivatableWidgetClass, [](EAsyncWidgetPushState, ActivatableWidgetT*) { ; });
	}

	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	TSharedPtr<FStreamableHandle> PushWidgetToLayerStackAsync(FGameplayTag LayerName,
	                                                          bool bSuspendInputUntilComplete,
	                                                          TSoftClassPtr<UCommonActivatableWidget> ActivatableWidgetClass,
	                                                          TFunction<void(EAsyncWidgetPushState, ActivatableWidgetT*)> StateFunc)
	{
		static_assert(TIsDerivedFrom<ActivatableWidgetT, UCommonActivatableWidget>::IsDerived, "Only CommonActivatableWidgets can be used here");

		// 是否需要暂停
		static FName NAME_PushingWidgetToLayer("PushingWidgetToLayer");
		const FName SuspendInputToken = bSuspendInputUntilComplete ? UUIHelperFunctions::SuspendInputForPlayer(GetOwningPlayer(), NAME_PushingWidgetToLayer) : NAME_None;

		TSharedPtr<FStreamableHandle> StreamingHandle = UAssetManager::Get().
		                                                GetStreamableManager().
		                                                RequestAsyncLoad(ActivatableWidgetClass.ToSoftObjectPath(), FStreamableDelegate::CreateWeakLambda(
			                                                                 this, [this, LayerName, ActivatableWidgetClass, StateFunc, SuspendInputToken]()
			                                                                 {
				                                                                 UUIHelperFunctions::ResumeInputForPlayer(GetOwningPlayer(), SuspendInputToken);

				                                                                 ActivatableWidgetT* Widget = PushWidgetToLayerStack<ActivatableWidgetT>(
					                                                                 LayerName, ActivatableWidgetClass.Get(), [StateFunc](ActivatableWidgetT& WidgetToInit)
					                                                                 {
						                                                                 StateFunc(EAsyncWidgetPushState::Initialize, &WidgetToInit);
					                                                                 });

				                                                                 StateFunc(EAsyncWidgetPushState::AfterPush, Widget);
			                                                                 })
		                                                );

		// 设置一个取消代理，这样当该处理器被取消时，我们可以继续输入。
		StreamingHandle->BindCancelDelegate(FStreamableDelegate::CreateWeakLambda(
				this, [this, StateFunc, SuspendInputToken]()
				{
					UUIHelperFunctions::ResumeInputForPlayer(GetOwningPlayer(), SuspendInputToken);
					StateFunc(EAsyncWidgetPushState::Canceled, nullptr);
				})
		);

		return StreamingHandle;
	}

	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	ActivatableWidgetT* PushWidgetToLayerStack(FGameplayTag LayerName, UClass* ActivatableWidgetClass)
	{
		return PushWidgetToLayerStack<ActivatableWidgetT>(LayerName, ActivatableWidgetClass, [](ActivatableWidgetT&) { ; });
	}

	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	ActivatableWidgetT* PushWidgetToLayerStack(FGameplayTag LayerName, UClass* ActivatableWidgetClass, TFunctionRef<void(ActivatableWidgetT&)> InitInstanceFunc)
	{
		static_assert(TIsDerivedFrom<ActivatableWidgetT, UCommonActivatableWidget>::IsDerived, "Only CommonActivatableWidgets can be used here");

		if (UCommonActivatableWidgetContainerBase* Layer = GetLayerFromTag(LayerName))
		{
			return Layer->AddWidget<ActivatableWidgetT>(ActivatableWidgetClass, InitInstanceFunc);
		}
		return nullptr;
	}

	// 在所有堆栈中寻找并删除指定的控件
	UE_API void FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget);

	// 获取对应的堆栈层
	UE_API UCommonActivatableWidgetContainerBase* GetLayerFromTag(FGameplayTag LayerName);

	/** 注册堆栈到缓存中. */
	UFUNCTION(BlueprintCallable, Category="AdvancedUI")
	UE_API void RegisterLayer(UPARAM(meta = (Categories = "AdvancedUI.UIStack")) FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget);

	UE_API void OnWidgetStackTransitioning(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning);

protected:
	virtual void NativeOnInitialized() override;

private:
	bool bIsDormant = false;

	// 跟踪所有暂停的输入令牌，这样多个异步UI都能加载，并且在所有UI期间正确暂停。
	TArray<FName> SuspendInputTokens;

	// 主布局的注册堆栈
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetContainerBase>> Layers;

	//默认的四个控件容器
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCommonActivatableWidgetStack> LayerStack_Modal;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCommonActivatableWidgetStack> LayerStack_GameMenu;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCommonActivatableWidgetStack> LayerStack_GameHUD;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCommonActivatableWidgetStack> LayerStack_Frontend;
};

#undef UE_API
