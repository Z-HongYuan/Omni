// Copyright © 2026 张鸿源. All Rights Reserved.


#include "System/GameUIPolicy.h"

#include "LogGameUI.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Framework/Application/SlateApplication.h"
#include "System/GameUIManager.h"
#include "Widgets/GameUIRootWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameUIPolicy)

UGameUIPolicy* UGameUIPolicy::GetGameUIPolicy(const UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		if (UGameInstance* GameInstance = World->GetGameInstance())
			if (UGameUIManager* GameUIManager = UGameInstance::GetSubsystem<UGameUIManager>(GameInstance))
				return GameUIManager->GetCurrentUIPolicy();

	return nullptr;
}

UWorld* UGameUIPolicy::GetWorld() const
{
	//原来的逻辑将会在编辑器下崩溃
	if (const UGameUIManager* OwningManager = GetOwningUIManager())
	{
		return OwningManager->GetGameInstance()->GetWorld();
	}
	return nullptr;
}

UGameUIManager* UGameUIPolicy::GetOwningUIManager() const
{
	return Cast<UGameUIManager>(GetOuter());
}

UGameUIRootWidget* UGameUIPolicy::GetRootLayoutWidget(const ULocalPlayer* LocalPlayer) const
{
	const FRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer);
	return LayoutInfo ? LayoutInfo->RootLayout : nullptr;
}

void UGameUIPolicy::RequestPrimaryControl(UGameUIRootWidget* Layout)
{
	if (LocalMultiplayerInteractionMode == ELocalMultiplayerViewMode::SingleToggle && Layout->IsDormant())
	{
		for (const FRootViewportLayoutInfo& LayoutInfo : RootViewportLayouts)
		{
			UGameUIRootWidget* RootLayout = LayoutInfo.RootLayout;
			if (RootLayout && !RootLayout->IsDormant())
			{
				RootLayout->SetIsDormant(true);
				break;
			}
		}
		Layout->SetIsDormant(false);
	}
}

void UGameUIPolicy::AddLayoutToViewport(ULocalPlayer* LocalPlayer, UGameUIRootWidget* Layout)
{
	UE_LOG(LogGameUI, Log, TEXT("[%s] is adding player [%s]'s root layout [%s] to the viewport"), *GetName(), *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));

	// 设置关联的 LocalPlayer 并添加到视口
	Layout->SetPlayerContext(FLocalPlayerContext(LocalPlayer));
	Layout->AddToPlayerScreen(1000);

	OnRootLayoutAddedToViewport(LocalPlayer, Layout);
}

void UGameUIPolicy::RemoveLayoutFromViewport(ULocalPlayer* LocalPlayer, UGameUIRootWidget* Layout)
{
	TWeakPtr<SWidget> LayoutSlateWidget = Layout->GetCachedWidget();
	if (LayoutSlateWidget.IsValid())
	{
		UE_LOG(LogGameUI, Log, TEXT("[%s] is removing player [%s]'s root layout [%s] from the viewport"), *GetName(), *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));

		Layout->RemoveFromParent();
		if (LayoutSlateWidget.IsValid())
		{
			UE_LOG(LogGameUI, Log, TEXT("Player [%s]'s root layout [%s] has been removed from the viewport, but other references to its underlying Slate widget still exist. Noting in case we leak it."), *GetNameSafe(LocalPlayer),
			       *GetNameSafe(Layout));
		}

		OnRootLayoutRemovedFromViewport(LocalPlayer, Layout);
	}
}

void UGameUIPolicy::OnRootLayoutAddedToViewport(ULocalPlayer* LocalPlayer, UGameUIRootWidget* Layout)
{
#if WITH_EDITOR
	if (GIsEditor && LocalPlayer->IsPrimaryPlayer())
	{
		// 所以我们的控制器可以在PIE中工作，无需点击视口
		FSlateApplication::Get().SetUserFocusToGameViewport(0);
	}
#endif
}

void UGameUIPolicy::OnRootLayoutRemovedFromViewport(ULocalPlayer* LocalPlayer, UGameUIRootWidget* Layout)
{
}

void UGameUIPolicy::OnRootLayoutReleased(ULocalPlayer* LocalPlayer, UGameUIRootWidget* Layout)
{
}

void UGameUIPolicy::CreateLayoutWidget(ULocalPlayer* LocalPlayer)
{
	if (APlayerController* PlayerController = LocalPlayer->GetPlayerController(GetWorld()))
	{
		TSubclassOf<UGameUIRootWidget> LayoutWidgetClass = GetLayoutWidgetClass();
		if (ensure(LayoutWidgetClass && !LayoutWidgetClass->HasAnyClassFlags(CLASS_Abstract)))
		{
			UGameUIRootWidget* NewLayoutObject = CreateWidget<UGameUIRootWidget>(PlayerController, LayoutWidgetClass);
			RootViewportLayouts.Emplace(LocalPlayer, NewLayoutObject, true);

			AddLayoutToViewport(LocalPlayer, NewLayoutObject);
		}
	}
}

TSubclassOf<UGameUIRootWidget> UGameUIPolicy::GetLayoutWidgetClass()
{
	return LayoutClass.LoadSynchronous();
}

void UGameUIPolicy::NotifyPlayerAdded(ULocalPlayer* LocalPlayer)
{
	// 在PC改变后重建一遍
	LocalPlayer->OnPlayerControllerChanged().AddWeakLambda(
		this, [this, LocalPlayer](APlayerController*)
		{
			NotifyPlayerRemoved(LocalPlayer);

			if (FRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
			{
				AddLayoutToViewport(LocalPlayer, LayoutInfo->RootLayout);
				LayoutInfo->bAddedToViewport = true;
			}
			else
			{
				CreateLayoutWidget(LocalPlayer);
			}
		}
	);

	// 直接添加
	if (FRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
	{
		AddLayoutToViewport(LocalPlayer, LayoutInfo->RootLayout);
		LayoutInfo->bAddedToViewport = true;
	}
	else
	{
		CreateLayoutWidget(LocalPlayer);
	}
}

void UGameUIPolicy::NotifyPlayerRemoved(ULocalPlayer* LocalPlayer)
{
	FRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer);
	if (!LayoutInfo) return;

	// 如果是有效数据,那么移除对应玩家的根控件
	RemoveLayoutFromViewport(LocalPlayer, LayoutInfo->RootLayout);
	LayoutInfo->bAddedToViewport = false;

	// 模式是单人全屏 且 被移除的玩家是次要玩家 那么休眠次要玩家,并且激活主要玩家的控件
	if (LocalMultiplayerInteractionMode == ELocalMultiplayerViewMode::SingleToggle && !LocalPlayer->IsPrimaryPlayer())
	{
		UGameUIRootWidget* RootLayout = LayoutInfo->RootLayout;
		if (RootLayout && !RootLayout->IsDormant())
		{
			RootLayout->SetIsDormant(true);
			for (const FRootViewportLayoutInfo& RootLayoutInfo : RootViewportLayouts)
			{
				if (RootLayoutInfo.LocalPlayer->IsPrimaryPlayer() && RootLayoutInfo.RootLayout)
				{
					RootLayoutInfo.RootLayout->SetIsDormant(false);
				}
			}
		}
	}
}

void UGameUIPolicy::NotifyPlayerDestroyed(ULocalPlayer* LocalPlayer)
{
	NotifyPlayerRemoved(LocalPlayer);
	LocalPlayer->OnPlayerControllerChanged().RemoveAll(this);
	const int32 LayoutInfoIdx = RootViewportLayouts.IndexOfByKey(LocalPlayer);
	if (LayoutInfoIdx != INDEX_NONE)
	{
		UGameUIRootWidget* Layout = RootViewportLayouts[LayoutInfoIdx].RootLayout;
		RootViewportLayouts.RemoveAt(LayoutInfoIdx);

		RemoveLayoutFromViewport(LocalPlayer, Layout);

		OnRootLayoutReleased(LocalPlayer, Layout);
	}
}
