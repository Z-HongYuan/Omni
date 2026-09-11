// Copyright © 2026 张鸿源. All Rights Reserved.


#include "System/UIManager.h"
#include "GameUISettings.h"
#include "LogGameUI.h"
#include "Engine/GameInstance.h"
#include "GameFramework/HUD.h"
#include "System/UIPolicy.h"
#include "Widgets/GameRootLayoutWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UIManager)

void UUIManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	//获取开发者设置
	const UGameUISettings* UISettings = GetDefault<UGameUISettings>();
	TSoftClassPtr<UUIPolicy> SoftPolicyClass = UISettings->DefaultUIPolicyClass;

	if (!CurrentPolicy && !SoftPolicyClass.IsNull())
	{
		TSubclassOf<UUIPolicy> PolicyClass = SoftPolicyClass.LoadSynchronous();
		if (PolicyClass && !PolicyClass->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
		{
			SwitchToPolicy(NewObject<UUIPolicy>(this, PolicyClass));
		}
		else
		{
			UE_LOG(LogGameUI, Warning, TEXT("UUIManager::Initialize: UI 策略 [%s] 加载失败或不是可实例化的类。"), *SoftPolicyClass.ToString());
		}
	}

	//添加Tick检测
	TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &ThisClass::Tick), 0.0f);

	// 直接挂钩在GameInstance的LocalPlayerEvent上
	if (UGameInstance* GI = GetGameInstance())
	{
		GI->OnLocalPlayerAddedEvent.AddUObject(this, &ThisClass::NotifyPlayerAdded);
		GI->OnLocalPlayerRemovedEvent.AddUObject(this, &ThisClass::NotifyPlayerDestroyed);
	}
	else
	{
		UE_LOG(LogGameUI, Error, TEXT("UUIManager::Initialize: 无法获取游戏实例，不能绑定本地玩家事件。"))
	}
}

void UUIManager::Deinitialize()
{
	Super::Deinitialize();

	SwitchToPolicy(nullptr);

	//移除Tick检测
	FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);

	// 直接挂钩在GameInstance的LocalPlayerEvent上
	if (UGameInstance* GI = GetGameInstance())
	{
		GI->OnLocalPlayerAddedEvent.RemoveAll(this);
		GI->OnLocalPlayerRemovedEvent.RemoveAll(this);
	}
	else
	{
		UE_LOG(LogGameUI, Error, TEXT("UUIManager::Deinitialize: 无法获取游戏实例，不能解除本地玩家事件。"))
	}
}

bool UUIManager::ShouldCreateSubsystem(UObject* Outer) const
{
	//继承链单例
	if (!CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance())
	{
		TArray<UClass*> ChildClasses;
		GetDerivedClasses(GetClass(), ChildClasses, false);

		return ChildClasses.Num() == 0;
	}

	return false;
}

void UUIManager::NotifyPlayerAdded(ULocalPlayer* LocalPlayer)
{
	if (ensure(LocalPlayer) && CurrentPolicy) CurrentPolicy->NotifyPlayerAdded(LocalPlayer);
}

void UUIManager::NotifyPlayerRemoved(ULocalPlayer* LocalPlayer)
{
	if (LocalPlayer && CurrentPolicy) CurrentPolicy->NotifyPlayerRemoved(LocalPlayer);
}

void UUIManager::NotifyPlayerDestroyed(ULocalPlayer* LocalPlayer)
{
	if (LocalPlayer && CurrentPolicy) CurrentPolicy->NotifyPlayerDestroyed(LocalPlayer);
}

void UUIManager::SwitchToPolicy(UUIPolicy* InPolicy)
{
	if (CurrentPolicy != InPolicy) CurrentPolicy = InPolicy;
}

bool UUIManager::Tick(float DeltaTime)
{
	SyncRootLayoutVisibilityToShowHUD();
	return true;
}

void UUIManager::SyncRootLayoutVisibilityToShowHUD()
{
	if (const UUIPolicy* Policy = GetCurrentUIPolicy())
	{
		for (const ULocalPlayer* LocalPlayer : GetGameInstance()->GetLocalPlayers())
		{
			bool bShouldShowUI = true;

			if (const APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld()))
			{
				const AHUD* HUD = PC->GetHUD();

				if (HUD && !HUD->bShowHUD)
				{
					bShouldShowUI = false;
				}
			}

			if (UGameRootLayoutWidget* RootLayout = Policy->GetRootLayoutWidget(LocalPlayer))
			{
				const ESlateVisibility DesiredVisibility = bShouldShowUI ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
				if (DesiredVisibility != RootLayout->GetVisibility())
				{
					RootLayout->SetVisibility(DesiredVisibility);
				}
			}
		}
	}
}
