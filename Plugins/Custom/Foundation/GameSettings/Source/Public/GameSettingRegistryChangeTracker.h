// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UObject/ObjectKey.h"
#include "UObject/WeakObjectPtrTemplates.h"

#define UE_API GAMESETTINGS_API

enum class EGameSettingChangeReason : uint8;

class UGameSetting;
class UGameSettingRegistry;
struct FObjectKey;

/** 跟踪注册表中的待保存更改，支持应用更改或恢复初始值；停止监听时解绑事件。 */
class FGameSettingRegistryChangeTracker : public FNoncopyable
{
public:
	UE_API FGameSettingRegistryChangeTracker();
	UE_API ~FGameSettingRegistryChangeTracker();

	UE_API void WatchRegistry(UGameSettingRegistry* InRegistry);
	UE_API void StopWatchingRegistry();

	UE_API void ApplyChanges();

	UE_API void RestoreToInitial();

	UE_API void ClearDirtyState();

	bool IsRestoringSettings() const { return bRestoringSettings; }
	bool HaveSettingsBeenChanged() const { return bSettingsChanged; }

private:
	UE_API void HandleSettingChanged(UGameSetting* Setting, EGameSettingChangeReason Reason);

	bool bSettingsChanged = false;
	bool bRestoringSettings = false;

	TWeakObjectPtr<UGameSettingRegistry> Registry;
	TMap<FObjectKey, TWeakObjectPtr<UGameSetting>> DirtySettings;
};

#undef UE_API
