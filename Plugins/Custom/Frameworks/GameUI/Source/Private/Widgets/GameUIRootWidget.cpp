// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Widgets/GameUIRootWidget.h"

#include "LogGameUI.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "System/GameUIGameplayTags.h"
#include "System/GameUIManager.h"
#include "System/GameUIPolicy.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameUIRootWidget)

UGameUIRootWidget* UGameUIRootWidget::GetRootLayoutWidgetForPrimaryPlayer(const UObject* WorldContextObject)
{
	UGameInstance* GameInstance = WorldContextObject ? UGameplayStatics::GetGameInstance(WorldContextObject) : nullptr;
	return GameInstance ? GetRootLayoutWidget(GameInstance->GetPrimaryPlayerController(false)) : nullptr;
}

UGameUIRootWidget* UGameUIRootWidget::GetRootLayoutWidget(APlayerController* PlayerController)
{
	return PlayerController ? GetRootLayoutWidget(PlayerController->GetLocalPlayer()) : nullptr;
}

UGameUIRootWidget* UGameUIRootWidget::GetRootLayoutWidget(ULocalPlayer* LocalPlayer)
{
	if (LocalPlayer)
		if (const UGameInstance* GameInstance = LocalPlayer->GetGameInstance())
			if (UGameUIManager* GameUIManager = GameInstance->GetSubsystem<UGameUIManager>())
				if (const UGameUIPolicy* Policy = GameUIManager->GetCurrentUIPolicy())
					if (UGameUIRootWidget* RootLayout = Policy->GetRootLayoutWidget(LocalPlayer))
						return RootLayout;

	return nullptr;
}

void UGameUIRootWidget::SetIsDormant(bool InDormant)
{
	if (bIsDormant == InDormant) return;

	// 记录所属玩家及休眠状态变化，便于定位控制权切换。
	const ULocalPlayer* LP = GetOwningLocalPlayer();
	const int32 PlayerId = LP ? LP->GetControllerId() : -1;
	const TCHAR* OldDormancyStr = bIsDormant ? TEXT("Dormant") : TEXT("Not-Dormant");
	const TCHAR* NewDormancyStr = InDormant ? TEXT("Dormant") : TEXT("Not-Dormant");
	const TCHAR* PrimaryPlayerStr = LP && LP->IsPrimaryPlayer() ? TEXT("[Primary]") : TEXT("[Non-Primary]");
	UE_LOG(LogGameUI, Display, TEXT("%s UGameUIRootWidget Dormancy changed for [%d] from [%s] to [%s]"), PrimaryPlayerStr, PlayerId, OldDormancyStr, NewDormancyStr);

	bIsDormant = InDormant;
	OnIsDormantChanged();
}

void UGameUIRootWidget::OnIsDormantChanged()
{
	// 留给派生类响应休眠变化；基类不直接改变可见性。
}

void UGameUIRootWidget::FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget)
{
	for (const auto& LayerKVP : Layers)
	{
		LayerKVP.Value->RemoveWidget(*ActivatableWidget);
	}
}

UCommonActivatableWidgetContainerBase* UGameUIRootWidget::GetLayerFromTag(FGameplayTag LayerName)
{
	return Layers.FindRef(LayerName);
}

void UGameUIRootWidget::RegisterLayer(FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget)
{
	if (!IsDesignTime())
	{
		LayerWidget->OnTransitioningChanged.AddUObject(this, &UGameUIRootWidget::OnWidgetStackTransitioning);
		// TODO: 支持非零过渡时长前需解决手柄焦点切换问题，目前使用零时长。
		LayerWidget->SetTransitionDuration(0.0);

		Layers.Add(LayerTag, LayerWidget);
	}
}

void UGameUIRootWidget::OnWidgetStackTransitioning(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning)
{
	// 过渡期间暂停输入；每次结束只归还对应的一枚令牌。
	if (bIsTransitioning)
	{
		const FName SuspendToken = UUIHelperFunctions::SuspendInputForPlayer(GetOwningLocalPlayer(), TEXT("GlobalStackTransion"));
		SuspendInputTokens.Add(SuspendToken);
	}
	else
	{
		if (ensure(SuspendInputTokens.Num() > 0))
		{
			const FName SuspendToken = SuspendInputTokens.Pop();
			UUIHelperFunctions::ResumeInputForPlayer(GetOwningLocalPlayer(), SuspendToken);
		}
	}
}

void UGameUIRootWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (LayerStack_Modal) RegisterLayer(GameUITags::TAG_GameUI_UIStack_Modal, LayerStack_Modal);
	if (LayerStack_GameMenu) RegisterLayer(GameUITags::TAG_GameUI_UIStack_GameMenu, LayerStack_GameMenu);
	if (LayerStack_GameHUD) RegisterLayer(GameUITags::TAG_GameUI_UIStack_GameHUD, LayerStack_GameHUD);
	if (LayerStack_Frontend) RegisterLayer(GameUITags::TAG_GameUI_UIStack_Frontend, LayerStack_Frontend);
}
