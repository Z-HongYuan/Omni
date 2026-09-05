// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Misc/TextFilterExpressionEvaluator.h"

#include "UObject/ObjectPtr.h"
#include "GameSettingFilterState.generated.h"

#define UE_API GAMESETTINGS_API

class ULocalPlayer;
class UGameSetting;
class UGameSettingCollection;

/** 设置为什么发生变化？ */
enum class EGameSettingChangeReason : uint8
{
	Change,
	DependencyChanged,
	ResetToDefault,
	RestoreToInitial,
};

/**
 * 筛选状态用于表示我们所支持的所有筛选方式。
 */
USTRUCT()
struct FGameSettingFilterState
{
	GENERATED_BODY()

public:
	UE_API FGameSettingFilterState();

	UPROPERTY()
	bool bIncludeDisabled = true;

	UPROPERTY()
	bool bIncludeHidden = false;

	UPROPERTY()
	bool bIncludeResetable = true;

	UPROPERTY()
	bool bIncludeNestedPages = false;

public:
	UE_API void SetSearchText(const FString& InSearchText);

	UE_API bool DoesSettingPassFilter(const UGameSetting& InSetting) const;

	UE_API void AddSettingToRootList(UGameSetting* InSetting);
	UE_API void AddSettingToAllowList(UGameSetting* InSetting);

	bool IsSettingInAllowList(const UGameSetting* InSetting) const
	{
		return SettingAllowList.Contains(InSetting);
	}

	const TArray<UGameSetting*>& GetSettingRootList() const { return SettingRootList; }

	bool IsSettingInRootList(const UGameSetting* InSetting) const
	{
		return SettingRootList.Contains(InSetting);
	}

private:
	FTextFilterExpressionEvaluator SearchTextEvaluator;

	UPROPERTY()
	TArray<TObjectPtr<UGameSetting>> SettingRootList;

	// 如果此列表非空，则只允许其中的设置通过
	UPROPERTY()
	TArray<TObjectPtr<UGameSetting>> SettingAllowList;
};

/**
 * 可编辑状态记录设置的当前可见性和启用状态，
 * 以及它进入该状态的原因。
 */
class FGameSettingEditableState
{
public:
	FGameSettingEditableState()
		: bVisible(true)
		  , bEnabled(true)
		  , bResetable(true)
		  , bHideFromAnalytics(false)
	{
	}

	bool IsVisible() const { return bVisible; }
	bool IsEnabled() const { return bEnabled; }
	bool IsResetable() const { return bResetable; }
	bool IsHiddenFromAnalytics() const { return bHideFromAnalytics; }
	const TArray<FText>& GetDisabledReasons() const { return DisabledReasons; }

#if !UE_BUILD_SHIPPING
	const TArray<FString>& GetHiddenReasons() const { return HiddenReasons; }
#endif

	const TArray<FString>& GetDisabledOptions() const { return DisabledOptions; }

	/** 隐藏该设置。你无需提供面向用户的原因，但必须指定一个开发者原因。 */
	UE_API void Hide(const FString& DevReason);

	/** 禁用该设置，你需要提供禁用此设置的原因。 */
	UE_API void Disable(const FText& Reason);

	/** 应对用户隐藏的离散选项。目前仅由家长控制使用。 */
	UE_API void DisableOption(const FString& Option);

	template <typename EnumType>
	void DisableEnumOption(EnumType InEnumValue)
	{
		DisableOption(StaticEnum<EnumType>()->GetNameStringByValue((int64)InEnumValue));
	}

	/**
	 * 防止用户在界面上将设置重置为默认值时重置此设置。
	 */
	UE_API void UnableToReset();

	/**
	 * 从分析中隐藏。例如，当你只是想避免噪音时可能需要这样做，例如针对特定平台的编辑条件，
	 * 在这些平台不存在时上报相关设置没有意义。
	 */
	void HideFromAnalytics() { bHideFromAnalytics = true; }

	/** 以所有可能的方式隐藏它：在视觉上隐藏，将其标记为不可重置，并从分析中隐藏。 */
	void Kill(const FString& DevReason)
	{
		Hide(DevReason);
		HideFromAnalytics();
		UnableToReset();
	}

private:
	uint8 bVisible : 1;
	uint8 bEnabled : 1;
	uint8 bResetable : 1;
	uint8 bHideFromAnalytics : 1;

	TArray<FString> DisabledOptions;

	TArray<FText> DisabledReasons;

#if !UE_BUILD_SHIPPING
	TArray<FString> HiddenReasons;
#endif
};

/**
 * 编辑条件可以监视游戏或其他设置的状态，
 * 并调整可见性。
 */
class FGameSettingEditCondition : public TSharedFromThis<FGameSettingEditCondition>
{
public:
	FGameSettingEditCondition()
	{
	}

	virtual ~FGameSettingEditCondition()
	{
	}

	DECLARE_EVENT_OneParam(FGameSettingEditCondition, FOnEditConditionChanged, bool);

	FOnEditConditionChanged OnEditConditionChangedEvent;

	/** 广播事件*/
	void BroadcastEditConditionChanged()
	{
		OnEditConditionChangedEvent.Broadcast(true);
	}

	/** 在设置初始化期间调用 */
	virtual void Initialize(const ULocalPlayer* InLocalPlayer)
	{
	}

	/** 当设置被"应用"时调用。 */
	virtual void SettingApplied(const ULocalPlayer* InLocalPlayer, UGameSetting* Setting) const
	{
	}

	/** 当设置被更改时调用。 */
	virtual void SettingChanged(const ULocalPlayer* InLocalPlayer, UGameSetting* Setting, EGameSettingChangeReason Reason) const
	{
	}

	/**
	 * 当设置需要重新评估编辑状态时调用。通常这是在依赖项发生变化，
	 * 或者此编辑条件发出 OnEditConditionChangedEvent 事件时触发的。
	 */
	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState& InOutEditState) const
	{
	}

	/** 为此编辑条件生成有用的调试文本。当事物不符合预期时很有帮助。 */
	virtual FString ToString() const { return TEXT(""); }
};

#undef UE_API
