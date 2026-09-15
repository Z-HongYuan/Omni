// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Assets/OmniAssetReferenceReminderSubsystem.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "CollectionManagerModule.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Notifications/NotificationManager.h"
#include "HAL/PlatformProcess.h"
#include "ICollectionContainer.h"
#include "ICollectionManager.h"
#include "Misc/Paths.h"
#include "OmniEditor/OmniEditorLogChannel.h"
#include "Widgets/Notifications/SNotificationList.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniAssetReferenceReminderSubsystem)

#define LOCTEXT_NAMESPACE "OmniAssetReferenceReminder"

void UOmniAssetReferenceReminderSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FCollectionManagerModule::GetModule();
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	AssetRenamedHandle = AssetRegistry.OnAssetRenamed().AddUObject(this, &ThisClass::HandleAssetRenamed);
}

void UOmniAssetReferenceReminderSubsystem::Deinitialize()
{
	if (const FAssetRegistryModule* AssetRegistryModule = FModuleManager::GetModulePtr<FAssetRegistryModule>(TEXT("AssetRegistry")))
	{
		AssetRegistryModule->Get().OnAssetRenamed().Remove(AssetRenamedHandle);
	}
	AssetRenamedHandle.Reset();
	FTSTicker::RemoveTicker(ReminderTickerHandle);
	ReminderTickerHandle.Reset();
	PendingAssets.Reset();

	Super::Deinitialize();
}

void UOmniAssetReferenceReminderSubsystem::HandleAssetRenamed(const FAssetData& AssetData, const FString& OldObjectPath)
{
	const FSoftObjectPath NewPath = AssetData.GetSoftObjectPath();
	const FSoftObjectPath OldPath(OldObjectPath);
	if (AssetData.IsRedirector() || OldPath == NewPath)
	{
		return;
	}

	static const FName CollectionName(TEXT("OuterRef"));
	const TSharedRef<ICollectionContainer>& Collections = FCollectionManagerModule::GetModule().Get().GetProjectCollectionContainer();
	TArray<FSoftObjectPath> MarkedAssets;
	Collections->GetAssetsInCollection(CollectionName, ECollectionShareType::CST_Shared, MarkedAssets);

	// 集合可能先于本回调更新路径，同时检查新旧路径以兼容通知顺序。
	if (!MarkedAssets.Contains(OldPath) && !MarkedAssets.Contains(NewPath))
	{
		return;
	}

	UE_LOG(LogOmniEditor, Warning,
	       TEXT("外部固定引用资产已移动或重命名：%s -> %s。请检查 C++ 固定名称、配置路径及 AssetManager 扫描范围，参见 Docs/FixedAssetReferences.md。"),
	       *OldObjectPath, *NewPath.ToString());

	if (!IsRunningCommandlet() && FSlateApplication::IsInitialized())
	{
		PendingAssets.Remove(OldPath);
		PendingAssets.Add(NewPath);
		if (!ReminderTickerHandle.IsValid())
		{
			// 延迟到编辑器继续处理消息时合并批量操作，避免为每个资产弹一条提示。
			ReminderTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
				FTickerDelegate::CreateUObject(this, &ThisClass::ShowPendingReminder), 0.2f);
		}
	}
}

bool UOmniAssetReferenceReminderSubsystem::ShowPendingReminder(float /*DeltaTime*/)
{
	ReminderTickerHandle.Reset();
	const FText Message = FText::Format(
		LOCTEXT("RenamedAssets", "已移动或重命名 {0} 个外部固定引用资产。\n请检查代码中的固定名称、配置路径及扫描范围。详细路径见输出日志。"),
		FText::AsNumber(PendingAssets.Num()));
	PendingAssets.Reset();

	FNotificationInfo Info(Message);
	Info.ExpireDuration = 15.0f;
	Info.HyperlinkText = LOCTEXT("OpenChecklist", "查看固定引用自查清单");
	Info.Hyperlink = FSimpleDelegate::CreateLambda([]
	{
		const FString GuidePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("Docs/FixedAssetReferences.md"));
		FPlatformProcess::LaunchFileInDefaultExternalApplication(*GuidePath);
	});
	FSlateNotificationManager::Get().AddNotification(Info);
	return false;
}

#undef LOCTEXT_NAMESPACE
