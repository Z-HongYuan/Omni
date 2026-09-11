// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "GameplayTagContainer.h"
#include "Engine/AssetManager.h"
#include "Engine/LocalPlayer.h"
#include "Engine/StreamableManager.h"
#include "UObject/StrongObjectPtr.h"
#include "HelperFunctions/UIHelperFunctions.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "GameUIRootWidget.generated.h"

#define UE_API GAMEUI_API

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
class UGameUIRootWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	// 从世界上下文获取主要玩家的根控件
	static UE_API UGameUIRootWidget* GetRootLayoutWidgetForPrimaryPlayer(const UObject* WorldContextObject);
	// 从 PC 获取 根控件
	static UE_API UGameUIRootWidget* GetRootLayoutWidget(APlayerController* PlayerController);
	// 从 本地玩家 获取 根控件
	static UE_API UGameUIRootWidget* GetRootLayoutWidget(ULocalPlayer* LocalPlayer);

	/** 更新根布局的休眠状态；具体显示行为由 OnIsDormantChanged 扩展。 */
	UE_API void SetIsDormant(bool InDormant);
	bool IsDormant() const { return bIsDormant; }
	UE_API virtual void OnIsDormantChanged();


	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	TSharedPtr<FStreamableHandle> PushWidgetToLayerStackAsync(FGameplayTag LayerName, bool bSuspendInputUntilComplete, TSoftClassPtr<UCommonActivatableWidget> ActivatableWidgetClass)
	{
		return PushWidgetToLayerStackAsync<ActivatableWidgetT>(LayerName, bSuspendInputUntilComplete, ActivatableWidgetClass, [](EAsyncWidgetPushState, ActivatableWidgetT*)
		{
		});
	}

	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	TSharedPtr<FStreamableHandle> PushWidgetToLayerStackAsync(FGameplayTag LayerName,
	                                                          bool bSuspendInputUntilComplete,
	                                                          TSoftClassPtr<UCommonActivatableWidget> ActivatableWidgetClass,
	                                                          TFunction<void(EAsyncWidgetPushState, ActivatableWidgetT*)> StateFunc)
	{
		static_assert(TIsDerivedFrom<ActivatableWidgetT, UCommonActivatableWidget>::IsDerived, "Only CommonActivatableWidgets can be used here");

		// 两个回调共享终态和输入令牌，布局失效后仍能完成清理。
		struct FAsyncPushOperation
		{
			TWeakObjectPtr<UGameUIRootWidget> RootLayout;
			TWeakObjectPtr<ULocalPlayer> LocalPlayer;
			FName InputToken = NAME_None;
			bool bFinished = false;
			TFunction<void(EAsyncWidgetPushState, ActivatableWidgetT*)> Callback;

			void ResumeInput()
			{
				UUIHelperFunctions::ResumeInputForPlayer(LocalPlayer.Get(), InputToken);
				InputToken = NAME_None;
			}

			void Finish(EAsyncWidgetPushState State, ActivatableWidgetT* Widget)
			{
				if (bFinished) return;
				bFinished = true;
				ResumeInput();
				Callback(State, Widget);
			}

			~FAsyncPushOperation() { ResumeInput(); }
		};

		if (ActivatableWidgetClass.IsNull())
		{
			StateFunc(EAsyncWidgetPushState::Canceled, nullptr);
			return nullptr;
		}

		const TSharedRef<FAsyncPushOperation> Operation = MakeShared<FAsyncPushOperation>();
		Operation->RootLayout = this;
		Operation->LocalPlayer = GetOwningLocalPlayer();
		Operation->Callback = MoveTemp(StateFunc);
		Operation->InputToken = bSuspendInputUntilComplete
			                        ? UUIHelperFunctions::SuspendInputForPlayer(Operation->LocalPlayer.Get(), TEXT("PushingWidgetToLayer"))
			                        : NAME_None;

		TSharedPtr<FStreamableHandle> StreamingHandle = UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
			ActivatableWidgetClass.ToSoftObjectPath(),
			FStreamableDelegateWithHandle::CreateLambda([Operation, LayerName, ActivatableWidgetClass](const TSharedPtr<FStreamableHandle>& Handle)
			{
				if (Operation->bFinished) return;
				Operation->ResumeInput();

				const TStrongObjectPtr<UGameUIRootWidget> RootLayout(Operation->RootLayout.Get());
				UClass* WidgetClass = ActivatableWidgetClass.Get();
				if (!RootLayout.IsValid() || !WidgetClass)
				{
					Operation->Finish(EAsyncWidgetPushState::Canceled, nullptr);
					return;
				}

				ActivatableWidgetT* Widget = RootLayout->PushWidgetToLayerStack<ActivatableWidgetT>(
					LayerName, WidgetClass, [Operation](ActivatableWidgetT& WidgetToInit)
					{
						if (!Operation->bFinished) Operation->Callback(EAsyncWidgetPushState::Initialize, &WidgetToInit);
					});

				// BeforePush 中也允许取消；此时撤下刚创建的控件，不再广播成功。
				if (Handle->WasCanceled() || Operation->bFinished)
				{
					if (Widget) RootLayout->FindAndRemoveWidgetFromLayer(Widget);
					Operation->Finish(EAsyncWidgetPushState::Canceled, nullptr);
					return;
				}

				Operation->Finish(Widget ? EAsyncWidgetPushState::AfterPush : EAsyncWidgetPushState::Canceled, Widget);
			}),
			FStreamableManager::DefaultAsyncLoadPriority, false, true);

		if (!StreamingHandle)
		{
			Operation->Finish(EAsyncWidgetPushState::Canceled, nullptr);
			return nullptr;
		}

		// 在启动加载前绑定取消回调；回调不依赖根布局对象的存活。
		StreamingHandle->BindCancelDelegate(FStreamableDelegate::CreateLambda([Operation]()
		{
			Operation->Finish(EAsyncWidgetPushState::Canceled, nullptr);
		}));
		StreamingHandle->StartStalledHandle();
		return StreamingHandle;
	}

	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	ActivatableWidgetT* PushWidgetToLayerStack(FGameplayTag LayerName, UClass* ActivatableWidgetClass)
	{
		return PushWidgetToLayerStack<ActivatableWidgetT>(LayerName, ActivatableWidgetClass, [](ActivatableWidgetT&)
		{
		});
	}

	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	ActivatableWidgetT* PushWidgetToLayerStack(FGameplayTag LayerName, UClass* ActivatableWidgetClass, TFunctionRef<void(ActivatableWidgetT&)> InitInstanceFunc)
	{
		static_assert(TIsDerivedFrom<ActivatableWidgetT, UCommonActivatableWidget>::IsDerived, "Only CommonActivatableWidgets can be used here");

		if (!ActivatableWidgetClass || ActivatableWidgetClass->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists)) return nullptr;

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
	UFUNCTION(BlueprintCallable, Category="GameUI")
	UE_API void RegisterLayer(UPARAM(meta = (Categories = "GameUI.UIStack")) FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget);

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
