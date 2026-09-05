// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSetting.h"

#include "GameSettingAction.generated.h"

#define UE_API GAMESETTINGS_API

//--------------------------------------
// UGameSettingAction
//--------------------------------------

class ULocalPlayer;

DECLARE_DELEGATE_TwoParams(UGameSettingCustomAction, UGameSetting* /*Setting*/, ULocalPlayer* /*LocalPlayer*/)

/**
 * 
 */
UCLASS(MinimalAPI)
class UGameSettingAction : public UGameSetting
{
	GENERATED_BODY()

public:
	UE_API UGameSettingAction();

public:
	DECLARE_EVENT_TwoParams(UGameSettingAction, FOnExecuteNamedAction, UGameSetting* /*Setting*/, FGameplayTag /*GameSettings_Action_Tag*/);

	FOnExecuteNamedAction OnExecuteNamedActionEvent;

public:
	FText GetActionText() const { return ActionText; }
	void SetActionText(FText Value) { ActionText = Value; }
#if !UE_BUILD_SHIPPING
	void SetActionText(const FString& Value) { SetActionText(FText::FromString(Value)); }
#endif

	FGameplayTag GetNamedAction() const { return NamedAction; }
	void SetNamedAction(FGameplayTag Value) { NamedAction = Value; }

	bool HasCustomAction() const { return CustomAction.IsBound(); }
	void SetCustomAction(UGameSettingCustomAction InAction) { CustomAction = InAction; }
	UE_API void SetCustomAction(TFunction<void(ULocalPlayer*)> InAction);

	/**
	 * 默认情况下，操作不会使设置变脏，因为大多数操作要么做的事情无法回退，
	 * 要么是像显示制作人员名单或 EULA 这样的内容。但如果情况并非如此，
	 * 请将此标志设为 true，以便在触发该操作时触发更改事件。
	 */
	void SetDoesActionDirtySettings(bool Value) { bDirtyAction = Value; }

	UE_API virtual void ExecuteAction();

protected:
	/** UGameSettingValue */
	UE_API virtual void OnInitialized() override;

protected:
	FText ActionText;
	FGameplayTag NamedAction;
	UGameSettingCustomAction CustomAction;
	bool bDirtyAction = false;
};

#undef UE_API
