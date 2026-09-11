// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Widgets/GameRootLayoutWidget.h"

#include "LogAdvancedUI.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "System/AdvancedUIGameplayTags.h"
#include "System/UIManager.h"
#include "System/UIPolicy.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameRootLayoutWidget)

UGameRootLayoutWidget* UGameRootLayoutWidget::GetRootLayoutWidgetForPrimaryPlayer(const UObject* WorldContextObject)
{
	UGameInstance* GameInstance = WorldContextObject ? UGameplayStatics::GetGameInstance(WorldContextObject) : nullptr;
	return GameInstance ? GetRootLayoutWidget(GameInstance->GetPrimaryPlayerController(false)) : nullptr;
}

UGameRootLayoutWidget* UGameRootLayoutWidget::GetRootLayoutWidget(APlayerController* PlayerController)
{
	return PlayerController ? GetRootLayoutWidget(PlayerController->GetLocalPlayer()) : nullptr;
}

UGameRootLayoutWidget* UGameRootLayoutWidget::GetRootLayoutWidget(ULocalPlayer* LocalPlayer)
{
	if (LocalPlayer)
		if (const UGameInstance* GameInstance = LocalPlayer->GetGameInstance())
			if (UUIManager* UIManager = GameInstance->GetSubsystem<UUIManager>())
				if (const UUIPolicy* Policy = UIManager->GetCurrentUIPolicy())
					if (UGameRootLayoutWidget* RootLayout = Policy->GetRootLayoutWidget(LocalPlayer))
						return RootLayout;

	return nullptr;
}

void UGameRootLayoutWidget::SetIsDormant(bool InDormant)
{
	if (bIsDormant == InDormant) return;

	// 打印 Log 信息
	const ULocalPlayer* LP = GetOwningLocalPlayer();
	const int32 PlayerId = LP ? LP->GetControllerId() : -1;
	const TCHAR* OldDormancyStr = bIsDormant ? TEXT("Dormant") : TEXT("Not-Dormant");
	const TCHAR* NewDormancyStr = InDormant ? TEXT("Dormant") : TEXT("Not-Dormant");
	const TCHAR* PrimaryPlayerStr = LP && LP->IsPrimaryPlayer() ? TEXT("[Primary]") : TEXT("[Non-Primary]");
	UE_LOG(LogAdvancedUI, Display, TEXT("%s UGameRootLayoutWidget Dormancy changed for [%d] from [%s] to [%s]"), PrimaryPlayerStr, PlayerId, OldDormancyStr, NewDormancyStr);

	bIsDormant = InDormant;
	OnIsDormantChanged();
}

void UGameRootLayoutWidget::OnIsDormantChanged()
{
	// 暂时禁用过渡动画，避免手柄焦点在切换控件时落到错误位置。
}

void UGameRootLayoutWidget::FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget)
{
	for (const auto& LayerKVP : Layers)
	{
		LayerKVP.Value->RemoveWidget(*ActivatableWidget);
	}
}

UCommonActivatableWidgetContainerBase* UGameRootLayoutWidget::GetLayerFromTag(FGameplayTag LayerName)
{
	return Layers.FindRef(LayerName);
}

void UGameRootLayoutWidget::RegisterLayer(FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget)
{
	if (!IsDesignTime())
	{
		LayerWidget->OnTransitioningChanged.AddUObject(this, &UGameRootLayoutWidget::OnWidgetStackTransitioning);
		// TODO: Consider allowing a transition duration, we currently set it to 0, because if it's not 0, the
		//       transition effect will cause focus to not transition properly to the new widgets when using
		//       gamepad always.
		LayerWidget->SetTransitionDuration(0.0);

		Layers.Add(LayerTag, LayerWidget);
	}
}

void UGameRootLayoutWidget::OnWidgetStackTransitioning(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning)
{
	// 在转换过程中禁用输入
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

void UGameRootLayoutWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (LayerStack_Modal) RegisterLayer(AdvancedUITags::TAG_AdvancedUI_UIStack_Modal, LayerStack_Modal);
	if (LayerStack_GameMenu) RegisterLayer(AdvancedUITags::TAG_AdvancedUI_UIStack_GameMenu, LayerStack_GameMenu);
	if (LayerStack_GameHUD) RegisterLayer(AdvancedUITags::TAG_AdvancedUI_UIStack_GameHUD, LayerStack_GameHUD);
	if (LayerStack_Frontend) RegisterLayer(AdvancedUITags::TAG_AdvancedUI_UIStack_Frontend, LayerStack_Frontend);
}
