// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Components/SlateWrapperTypes.h"
#include "GameSettingFilterState.h"
#include "GameplayTagContainer.h"

#include "GameSetting.generated.h"

#define UE_API GAMESETTINGS_API

class ULocalPlayer;
class UGameSettingRegistry;

//--------------------------------------
// UGameSetting
//--------------------------------------

DECLARE_DELEGATE_RetVal_OneParam(FText, FGetGameSettingsDetails, ULocalPlayer& /*InLocalPlayer*/);

/**
 * 
 */
UCLASS(MinimalAPI, Abstract, BlueprintType)
class UGameSetting : public UObject
{
	GENERATED_BODY()

public:
	UGameSetting()
	{
	}

public:
	DECLARE_EVENT_TwoParams(UGameSetting, FOnSettingChanged, UGameSetting* /*InSetting*/, EGameSettingChangeReason /*InChangeReason*/);

	DECLARE_EVENT_OneParam(UGameSetting, FOnSettingApplied, UGameSetting* /*InSetting*/);

	DECLARE_EVENT_OneParam(UGameSetting, FOnSettingEditConditionChanged, UGameSetting* /*InSetting*/);

	FOnSettingChanged OnSettingChangedEvent;
	FOnSettingApplied OnSettingAppliedEvent;
	FOnSettingEditConditionChanged OnSettingEditConditionChangedEvent;

public:
	/**
	 * 获取此设置的非本地化开发名称。该名称应保持不变，并作为此设置在
	 * 设置注册表中的唯一标识符。
	 */
	UFUNCTION(BlueprintCallable)
	FName GetDevName() const { return DevName; }

	void SetDevName(const FName& Value) { DevName = Value; }

	bool GetAdjustListViewPostRefresh() const { return bAdjustListViewPostRefresh; }
	void SetAdjustListViewPostRefresh(const bool Value) { bAdjustListViewPostRefresh = Value; }

	UFUNCTION(BlueprintCallable)
	FText GetDisplayName() const { return DisplayName; }

	void SetDisplayName(const FText& Value) { DisplayName = Value; }
#if !UE_BUILD_SHIPPING
	void SetDisplayName(const FString& Value) { SetDisplayName(FText::FromString(Value)); }
#endif
	UFUNCTION(BlueprintCallable)
	ESlateVisibility GetDisplayNameVisibility() { return DisplayNameVisibility; }

	void SetNameDisplayVisibility(ESlateVisibility InVisibility) { DisplayNameVisibility = InVisibility; }

	UFUNCTION(BlueprintCallable)
	FText GetDescriptionRichText() const { return DescriptionRichText; }

	void SetDescriptionRichText(const FText& Value)
	{
		DescriptionRichText = Value;
		InvalidateSearchableText();
	}
#if !UE_BUILD_SHIPPING
	/** 此版本用于作弊指令和其他非发布版本的项目，这些内容无需本地化文本。我们不允许在发布版本中使用它，以防止引入未本地化的文本。 */
	void SetDescriptionRichText(const FString& Value) { SetDescriptionRichText(FText::FromString(Value)); }
#endif

	UFUNCTION(BlueprintCallable)
	const FGameplayTagContainer& GetTags() const { return Tags; }

	void AddTag(const FGameplayTag& TagToAdd) { Tags.AddTag(TagToAdd); }

	void SetRegistry(UGameSettingRegistry* InOwningRegistry) { OwningRegistry = InOwningRegistry; }

	/** 获取该设置的描述对应的可搜索纯文本。 */
	UE_API const FString& GetDescriptionPlainText() const;

	/** 初始化设置，为其指定所属的本地玩家。容器会自动初始化添加到其中的设置。 */
	UE_API void Initialize(ULocalPlayer* InLocalPlayer);

	/** 获取此设置所属的本地玩家——所有已初始化的设置都会拥有它。 */
	ULocalPlayer* GetOwningLocalPlayer() const { return LocalPlayer; }

	/** 设置动态详情回调，我们在构建描述面板时会查询它。此文本不可搜索。*/
	void SetDynamicDetails(const FGetGameSettingsDetails& InDynamicDetails) { DynamicDetails = InDynamicDetails; }

	/**
	 * 获取有关此设置的动态详情。这些信息可能包括诸如账户还剩多少次退款，
	 * 或账户号码等内容。
	 */
	UFUNCTION(BlueprintCallable)
	UE_API FText GetDynamicDetails() const;

	UFUNCTION(BlueprintCallable)
	FText GetWarningRichText() const { return WarningRichText; }

	void SetWarningRichText(const FText& Value)
	{
		WarningRichText = Value;
		InvalidateSearchableText();
	}
#if !UE_BUILD_SHIPPING
	/** 此版本用于作弊指令和其他非发布版本的项目，这些内容无需本地化文本。我们不允许在发布版本中使用它，以防止引入未本地化的文本。 */
	void SetWarningRichText(const FString& Value) { SetWarningRichText(FText::FromString(Value)); }
#endif

	/**
	 * 基于编辑条件的当前状态以及任何附加的筛选状态，
	 * 获取此属性的编辑状态。
	 */
	const FGameSettingEditableState& GetEditState() const { return EditableStateCache; }

	/** 向此设置添加一个新的编辑条件，使你可以控制此设置的可见性和可编辑性。 */
	UE_API void AddEditCondition(const TSharedRef<FGameSettingEditCondition>& InEditCondition);

	/** 添加设置依赖，如果这些设置发生变化，我们将重新评估此设置的编辑条件。 */
	UE_API void AddEditDependency(UGameSetting* DependencySetting);

	/** 拥有该设置的父对象，大多数情况下是集合，但对于顶级设置来说是注册表。 */
	UE_API void SetSettingParent(UGameSetting* InSettingParent);
	UGameSetting* GetSettingParent() const { return SettingParent; }

	/** 此设置是否应上报到分析系统。 */
	bool GetIsReportedToAnalytics() const { return bReportAnalytics; }
	void SetIsReportedToAnalytics(bool bReport) { bReportAnalytics = bReport; }

	/** 获取此设置的分析值。 */
	virtual FString GetAnalyticsValue() const { return TEXT(""); }

	/**
	 * 某些设置可能需要异步时间才能完成初始化。设置系统会等待
	 * 所有设置都就绪后再显示该设置。
	 */
	bool IsReady() const { return bReady; }

	/**
	 * 任何设置都可以有子设置，这样我们就可以支持"集合"或"操作"的可能性，
	 * 它们对用户不直接可见，但通过某种方式被设置，并且需要有初始值和恢复值。
	 * 在这种情况下，你可能需要在操作子类中包含内部设置，这些设置在另一个界面中被设置，
	 * 但永远不会直接列在设置面板上。
	 */
	virtual TArray<UGameSetting*> GetChildSettings() { return TArray<UGameSetting*>(); }

	/**
	 * 刷新设置的编辑状态，并通知状态已发生变化，以便当前正在查看此设置的所有 UI
	 * 都能用新选项或其他内容进行更新。
	 */
	UE_API void RefreshEditableState(bool bNotifyEditConditionsChanged = true);

	/**
	 * 我们期望设置会立即更改实时值，但偶尔会有一些特殊设置
	 * 会立即存储到临时位置，但直到稍后才真正应用，
	 * 例如选择新的分辨率。
	 */
	UE_API void Apply();

	/** 获取拥有这些设置的本地玩家所在的当前世界。 */
	UE_API virtual UWorld* GetWorld() const override;

protected:
	/**  */
	UE_API virtual void Startup();
	UE_API void StartupComplete();

	UE_API virtual void OnInitialized();
	UE_API virtual void OnApply();
	UE_API virtual void OnGatherEditState(FGameSettingEditableState& InOutEditState) const;
	UE_API virtual void OnDependencyChanged();

	/**  */
	UE_API virtual FText GetDynamicDetailsInternal() const;

	/** */
	UE_API void HandleEditDependencyChanged(UGameSetting* DependencySetting, EGameSettingChangeReason Reason);
	UE_API void HandleEditDependencyChanged(UGameSetting* DependencySetting);

	/** 如果纯搜索文本已失效，则重新生成它。 */
	UE_API void RefreshPlainText() const;
	void InvalidateSearchableText() { bRefreshPlainSearchableText = true; }

	/** 通知设置已更改 */
	UE_API void NotifySettingChanged(EGameSettingChangeReason Reason);
	UE_API virtual void OnSettingChanged(EGameSettingChangeReason Reason);

	/** 通知设置的编辑条件已更改。这可能意味着它现在不可见、被禁用，或者选项以某种有意义的方式发生了变化。 */
	UE_API void NotifyEditConditionsChanged();
	UE_API virtual void OnEditConditionsChanged();

	/**  */
	UE_API FGameSettingEditableState ComputeEditableState() const;

protected:
	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> LocalPlayer;

	UPROPERTY(Transient)
	TObjectPtr<UGameSetting> SettingParent;

	UPROPERTY(Transient)
	TObjectPtr<UGameSettingRegistry> OwningRegistry;

	FName DevName;
	FText DisplayName;
	ESlateVisibility DisplayNameVisibility = ESlateVisibility::SelfHitTestInvisible;
	FText DescriptionRichText;
	FText WarningRichText;

	/** 此设置的标签集合。这些标签可以只是 UI 用于执行不同操作的任意标志。 */
	FGameplayTagContainer Tags;

	FGetGameSettingsDetails DynamicDetails;

	/** 此设置的所有编辑条件。 */
	TArray<TSharedRef<FGameSettingEditCondition>> EditConditions;

	class FStringCultureCache
	{
		FStringCultureCache(TFunction<FString()> InStringGetter);

		void Invalidate();

		FString Get() const;

	private:
		mutable FString StringCache;
		mutable FCultureRef Culture;
		TFunction<FString()> StringGetter;
	};

	/** 当文本发生变化时，我们会使可搜索文本失效。 */
	mutable bool bRefreshPlainSearchableText = true;
	/** 当我们为设置设置富文本时，会自动生成纯文本。 */
	mutable FString AutoGenerated_DescriptionPlainText;

	/** 作为分析的一部分上报，默认情况下除了 GameSettingValues 外，没有设置会上报。 */
	bool bReportAnalytics = false;

private:
	/** 大多数设置立即可用，但有些设置可能需要启动时间后才能安全地调用其函数。 */
	bool bReady = false;

	/** 防止在宣布设置已更改时出现重入问题。 */
	bool bOnSettingChangedEventGuard = false;

	/** 防止在宣布设置的编辑条件已更改时出现重入问题。 */
	bool bOnEditConditionsChangedEventGuard = false;

	/**  */
	bool bAdjustListViewPostRefresh = true;

	/** 我们会在设置的可编辑状态发生变化时缓存它，而不是在每次需要时重新处理。 */
	FGameSettingEditableState EditableStateCache;
};

#undef UE_API
