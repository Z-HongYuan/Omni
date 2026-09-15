// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "EditorSubsystem.h"
#include "UObject/SoftObjectPath.h"
#include "OmniAssetReferenceReminderSubsystem.generated.h"

struct FAssetData;

#define UE_API OMNIEDITOR_API

/** 移动或重命名 OuterRef 集合中的资产后，提醒检查代码和配置。 */
UCLASS(MinimalAPI)
class UOmniAssetReferenceReminderSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;

private:
	void HandleAssetRenamed(const FAssetData& AssetData, const FString& OldObjectPath);
	bool ShowPendingReminder(float DeltaTime);

	FDelegateHandle AssetRenamedHandle;
	FTSTicker::FDelegateHandle ReminderTickerHandle;
	TSet<FSoftObjectPath> PendingAssets;
};

#undef UE_API
