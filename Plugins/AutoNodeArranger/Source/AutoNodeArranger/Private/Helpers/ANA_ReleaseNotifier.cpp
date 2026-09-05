// Copyright 2024 bstt, Inc. All Rights Reserved.

#include "ANA_ReleaseNotifier.h"
#include "../Config/ANA_EditorConfig.h"
#include "../Debug/Log.h"
#include "../Helpers/ANA_CacheHelper.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Subsystems/AssetEditorSubsystem.h"

#include "Debug/Debug.h"

#define ANA_NEW_RELEASE_VERSION 501

void ANA_ReleaseNotifier::RegisterCheckVersion()
{
	static FDelegateHandle onAssetEditorOpenedHandle;

	if (UAssetEditorSubsystem* assetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
		onAssetEditorOpenedHandle = assetEditorSubsystem->OnAssetEditorOpened().AddLambda(
			[this, assetEditorSubsystem](UObject* Object)
			{
				CheckVersion();
				assetEditorSubsystem->OnAssetEditorOpened().Remove(onAssetEditorOpenedHandle);
			});
}

void ANA_ReleaseNotifier::CheckVersion()
{
	ANA_ReleaseNotifierData cacheData = ANA_CacheHelper::Get().GetReleaseNotifierData();

	if (cacheData.version >= ANA_NEW_RELEASE_VERSION) return;
	if (FDateTime::UtcNow() > FDateTime(2024, 8, 01)) return;
	ShowNotification(EANA_ReleaseStep::AUTO_ARRANGE_THANKS);
}

void ANA_ReleaseNotifier::ShowNotification(EANA_ReleaseStep completeActionType_)
{
	completeActionType = completeActionType_;

	switch (completeActionType)
	{
	case EANA_ReleaseStep::AUTO_ARRANGE_THANKS:
		ShowAutoArrangeThanksNotification();
		break;
	case EANA_ReleaseStep::AUTO_ARRANGE_ISSUE:
		ShowAutoArrangeIssueNotification();
		break;
	case EANA_ReleaseStep::AUTO_ARRANGE_BETA:
		ShowAutoArrangeBetaNotification();
		break;
	case EANA_ReleaseStep::NONE:
		ShowNoneNotification();
		break;
	default:
		break;
	}
}

void ANA_ReleaseNotifier::ShowAutoArrangeThanksNotification()
{
	if (releaseNotifPtr.IsValid()) return;
	FString message = FString("Auto Node Arranger Thanks");
	FString subText = FString("First of all, thank you for continuing to use my plugin.\n"
							  "I try my best to take the time to improve it.\n"
							  "I hope you enjoy using it and continue to do so.\n");

	FNotificationInfo releaseNotif(FText::FromString(message));
	releaseNotif.SubText = FText::FromString(subText);
	releaseNotif.FadeOutDuration = 0.2f;
	releaseNotif.bFireAndForget = false;

	releaseNotif.ButtonDetails.Add(FNotificationButtonInfo(FText::FromString("Ok"),
		FText::FromString("Ok"),
		FSimpleDelegate::CreateLambda(
			[this]() mutable
			{
				releaseNotifPtr.Pin()->SetFadeOutDuration(0.f);
				releaseNotifPtr.Pin()->Fadeout();
				releaseNotifPtr = nullptr;
				ShowNotification(EANA_ReleaseStep::AUTO_ARRANGE_ISSUE);
			}),
		SNotificationItem::ECompletionState::CS_None));

	releaseNotifPtr = FSlateNotificationManager::Get().AddNotification(releaseNotif);

	UE_LOG(LogAutoNodeArranger, Log, TEXT("%s\n%s"), *message, *subText);
}

void ANA_ReleaseNotifier::ShowAutoArrangeIssueNotification()
{
	if (releaseNotifPtr.IsValid()) return;
	FString message = FString("Auto Node Arranger Issues");
	FString subText = FString("The github issue section is finally opened. You can toggle the quick export a graph by pressing "
							  "(ctrl + shift + space).");

	FNotificationInfo releaseNotif(FText::FromString(message));
	releaseNotif.SubText = FText::FromString(subText);
	releaseNotif.FadeOutDuration = 0.2f;
	releaseNotif.bFireAndForget = false;

	releaseNotif.Hyperlink = FSimpleDelegate::CreateLambda(
		[]()
		{
			const FString DocsURL = TEXT("https://github.com/bstt/AutoNodeArranger#5-raise-issue-1");
			FPlatformProcess::LaunchURL(*DocsURL, nullptr, nullptr);
		});
	releaseNotif.HyperlinkText = FText::FromString("More details...");

	releaseNotif.ButtonDetails.Add(FNotificationButtonInfo(FText::FromString("Ok"),
		FText::FromString("Ok"),
		FSimpleDelegate::CreateLambda(
			[this]() mutable
			{
				releaseNotifPtr.Pin()->SetFadeOutDuration(0.f);
				releaseNotifPtr.Pin()->Fadeout();
				releaseNotifPtr = nullptr;
				ShowNotification(EANA_ReleaseStep::AUTO_ARRANGE_BETA);
			}),
		SNotificationItem::ECompletionState::CS_None));

	releaseNotifPtr = FSlateNotificationManager::Get().AddNotification(releaseNotif);

	UE_LOG(LogAutoNodeArranger, Log, TEXT("%s\n%s"), *message, *subText);
}

void setBetaConfig(bool bUseBeta)
{
	UANA_EditorConfig::Get()->bUseBeta = bUseBeta;
	UANA_EditorConfig::Get()->SaveConfig();
	// update cache version
	ANA_ReleaseNotifierData cacheData = ANA_CacheHelper::Get().GetReleaseNotifierData();
	cacheData.version = ANA_NEW_RELEASE_VERSION;
	ANA_CacheHelper::Get().SetReleaseNotifierData(cacheData);
	FNotificationInfo notifInfo(NSLOCTEXT("AutoNodeArranger", "BetaUsage", ""));
	FString message = FString("Beta ") + (UANA_EditorConfig::Get()->bUseBeta ? " enabled" : "disabled")
					  + FString(".\nYou can change the value in the Editor Preferences.");
	notifInfo.Text = FText::FromString(message);
	notifInfo.ExpireDuration = 3.0f;
	FSlateNotificationManager::Get().AddNotification(notifInfo);
	UE_LOG(LogAutoNodeArranger, Warning, TEXT("%s"), *message);
}

void ANA_ReleaseNotifier::ShowAutoArrangeBetaNotification()
{
	if (releaseNotifPtr.IsValid()) return;
	FString message = FString("Auto Node Arranger Beta");
	FString subText = FString("A beta version of the Arrangement Straight is available.\n"
							  "It is not stable for now, but you can try it.\n"
							  "Do you want to try it?");

	FNotificationInfo releaseNotif(FText::FromString(message));
	releaseNotif.SubText = FText::FromString(subText);
	releaseNotif.FadeOutDuration = 0.2f;
	releaseNotif.bFireAndForget = false;

	releaseNotif.ButtonDetails.Add(FNotificationButtonInfo(FText::FromString("Yes"),
		FText::FromString("Yes"),
		FSimpleDelegate::CreateLambda(
			[this]()
			{
				releaseNotifPtr.Pin()->SetFadeOutDuration(0.f);
				releaseNotifPtr.Pin()->Fadeout();
				releaseNotifPtr = nullptr;
				setBetaConfig(true);
			}),
		SNotificationItem::ECompletionState::CS_None));
	releaseNotif.ButtonDetails.Add(FNotificationButtonInfo(FText::FromString("No"),
		FText::FromString("No"),
		FSimpleDelegate::CreateLambda(
			[this]()
			{
				releaseNotifPtr.Pin()->SetFadeOutDuration(0.f);
				releaseNotifPtr.Pin()->Fadeout();
				releaseNotifPtr = nullptr;
				setBetaConfig(false);
			}),
		SNotificationItem::ECompletionState::CS_None));

	releaseNotifPtr = FSlateNotificationManager::Get().AddNotification(releaseNotif);

	UE_LOG(LogAutoNodeArranger, Log, TEXT("%s\n%s"), *message, *subText);
}

void ANA_ReleaseNotifier::ShowNoneNotification()
{
	// update cache version
	ANA_ReleaseNotifierData cacheData = ANA_CacheHelper::Get().GetReleaseNotifierData();
	cacheData.version = ANA_NEW_RELEASE_VERSION;
	ANA_CacheHelper::Get().SetReleaseNotifierData(cacheData);
}
