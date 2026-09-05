// Copyright © 2026 张鸿源. All Rights Reserved.


#include "System/UIManager.h"
#include "AdvancedUISettings.h"
#include "LogAdvancedUI.h"
#include "Engine/GameInstance.h"
#include "GameFramework/HUD.h"
#include "System/UIPolicy.h"
#include "Widgets/GameRootLayoutWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UIManager)

void UUIManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	//获取开发者设置
	const UAdvancedUISettings* UISettings = GetDefault<UAdvancedUISettings>();
	TSoftClassPtr<UUIPolicy> SoftPolicyClass = UISettings->DefaultUIPolicyClass;

	if (!CurrentPolicy && !SoftPolicyClass.IsNull())
	{
		TSubclassOf<UUIPolicy> PolicyClass = SoftPolicyClass.LoadSynchronous();
		// 开局切换一次,相当于初始化了
		SwitchToPolicy(NewObject<UUIPolicy>(this, PolicyClass));
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
		UE_LOG(LogAdvancedUI, Error, TEXT("UUIManager::Initialize Not Get GameInstance,Can't Broadcast LocalPlayer Add or Remove Event"))
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
		UE_LOG(LogAdvancedUI, Error, TEXT("UUIManager::Initialize Not Get GameInstance,Can't Broadcast LocalPlayer Add or Remove Event"))
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
