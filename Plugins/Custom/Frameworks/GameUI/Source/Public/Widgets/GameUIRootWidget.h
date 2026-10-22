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

/** 异步控件推送的回调阶段。 */
enum class EAsyncWidgetPushState : uint8
{
	Canceled,
	Initialize,
	AfterPush
};

/** 本地玩家的 UI 根布局，管理分层堆栈及控件推送。 */
UCLASS(MinimalAPI, Abstract, meta = (DisableNativeTick))
class UGameUIRootWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/** 从世界上下文获取主要本地玩家的根布局。 */
	static UE_API UGameUIRootWidget* GetRootLayoutWidgetForPrimaryPlayer(const UObject* WorldContextObject);
	/** 获取控制器所属本地玩家的根布局。 */
	static UE_API UGameUIRootWidget* GetRootLayoutWidget(APlayerController* PlayerController);
	/** 获取本地玩家的根布局，策略或布局未就绪时返回空指针。 */
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

	/** 异步加载并推送控件；回调接收初始化及终态，取消或失败均归还输入令牌。 */
	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	TSharedPtr<FStreamableHandle> PushWidgetToLayerStackAsync(FGameplayTag LayerName,
	                                                          bool bSuspendInputUntilComplete,
	                                                          TSoftClassPtr<UCommonActivatableWidget> ActivatableWidgetClass,
	                                                          TFunction<void(EAsyncWidgetPushState, ActivatableWidgetT*)> StateFunc)
	{
		static_assert(TIsDerivedFrom<ActivatableWidgetT, UCommonActivatableWidget>::IsDerived, "Only CommonActivatableWidgets can be used here");

		/** 加载和取消回调共享终态及输入令牌，布局失效后仍负责归还输入。 */
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

	/** 从此根布局的所有堆栈中移除指定控件。 */
	UE_API void FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget);

	/** 按标签查询已注册的堆栈层。 */
	UE_API UCommonActivatableWidgetContainerBase* GetLayerFromTag(FGameplayTag LayerName);

	/** 注册堆栈，并监听过渡状态以管理输入暂停。 */
	UFUNCTION(BlueprintCallable, Category="GameUI")
	UE_API void RegisterLayer(UPARAM(meta = (Categories = "GameUI.UIStack")) FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget);

	UE_API void OnWidgetStackTransitioning(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning);

protected:
	virtual void NativeOnInitialized() override;

private:
	bool bIsDormant = false;

	/** 跟踪堆栈过渡产生的输入暂停令牌，过渡结束时逐个归还。 */
	TArray<FName> SuspendInputTokens;

	/** 按标签保存已注册的 UI 堆栈。 */
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetContainerBase>> Layers;

	// 可选的默认 UI 层容器。
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
