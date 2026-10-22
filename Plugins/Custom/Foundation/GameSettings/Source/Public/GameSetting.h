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


DECLARE_DELEGATE_RetVal_OneParam(FText, FGetGameSettingsDetails, ULocalPlayer& /*InLocalPlayer*/);

/** 管理设置的玩家上下文、初始化、编辑条件和变更通知。 */
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
	/** 返回设置注册表内唯一且稳定的开发名称；该名称不参与本地化。 */
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
	/** 仅供非 Shipping 构建设置无需本地化的调试文本。 */
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

	/** 返回初始化时指定的本地玩家；初始化前可能为空。 */
	ULocalPlayer* GetOwningLocalPlayer() const { return LocalPlayer; }

	/** 设置详情面板查询的动态文本回调；该文本不参与搜索。 */
	void SetDynamicDetails(const FGetGameSettingsDetails& InDynamicDetails) { DynamicDetails = InDynamicDetails; }

	/** 查询当前动态详情文本，并按调试开关追加设置标识。 */
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
	/** 仅供非 Shipping 构建设置无需本地化的调试文本。 */
	void SetWarningRichText(const FString& Value) { SetWarningRichText(FText::FromString(Value)); }
#endif

	/** 返回最近一次计算的编辑状态缓存。 */
	const FGameSettingEditableState& GetEditState() const { return EditableStateCache; }

	/** 向此设置添加一个新的编辑条件，使你可以控制此设置的可见性和可编辑性。 */
	UE_API void AddEditCondition(const TSharedRef<FGameSettingEditCondition>& InEditCondition);

	/** 添加设置依赖，如果这些设置发生变化，我们将重新评估此设置的编辑条件。 */
	UE_API void AddEditDependency(UGameSetting* DependencySetting);

	/** 设置所属的父级设置；顶层设置可以没有父级。 */
	UE_API void SetSettingParent(UGameSetting* InSettingParent);
	UGameSetting* GetSettingParent() const { return SettingParent; }

	/** 此设置是否应上报到分析系统。 */
	bool GetIsReportedToAnalytics() const { return bReportAnalytics; }
	void SetIsReportedToAnalytics(bool bReport) { bReportAnalytics = bReport; }

	/** 获取此设置的分析值。 */
	virtual FString GetAnalyticsValue() const { return TEXT(""); }

	/** 返回异步初始化是否完成；未就绪时界面应继续等待。 */
	bool IsReady() const { return bReady; }

	/** 返回参与初始化和注册的子设置；子设置也可以仅供内部操作使用。 */
	virtual TArray<UGameSetting*> GetChildSettings() { return TArray<UGameSetting*>(); }

	/** 重新计算可编辑状态，并按参数决定是否通知界面更新。 */
	UE_API void RefreshEditableState(bool bNotifyEditConditionsChanged = true);

	/** 应用当前设置并发送应用通知；子类可在此提交暂存的值。 */
	UE_API void Apply();

	/** 获取拥有这些设置的本地玩家所在的当前世界。 */
	UE_API virtual UWorld* GetWorld() const override;

protected:
	/** 开始初始化；异步子类完成后必须调用 StartupComplete。 */
	UE_API virtual void Startup();
	UE_API void StartupComplete();

	UE_API virtual void OnInitialized();
	UE_API virtual void OnApply();
	UE_API virtual void OnGatherEditState(FGameSettingEditableState& InOutEditState) const;
	UE_API virtual void OnDependencyChanged();

	UE_API virtual FText GetDynamicDetailsInternal() const;

	/** 依赖设置变化时重新计算编辑状态，并按需要转发依赖变更通知。 */
	UE_API void HandleEditDependencyChanged(UGameSetting* DependencySetting, EGameSettingChangeReason Reason);
	UE_API void HandleEditDependencyChanged(UGameSetting* DependencySetting);

	/** 如果纯搜索文本已失效，则重新生成它。 */
	UE_API void RefreshPlainText() const;
	void InvalidateSearchableText() { bRefreshPlainSearchableText = true; }

	/** 通知编辑条件和订阅者：设置值已更改。 */
	UE_API void NotifySettingChanged(EGameSettingChangeReason Reason);
	UE_API virtual void OnSettingChanged(EGameSettingChangeReason Reason);

	/** 通知设置的编辑条件已更改。这可能意味着它现在不可见、被禁用，或者选项以某种有意义的方式发生了变化。 */
	UE_API void NotifyEditConditionsChanged();
	UE_API virtual void OnEditConditionsChanged();

	/** 合并设置自身规则及所有编辑条件，生成当前编辑状态。 */
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

	/** 描述文本变化后置为 true，下一次搜索查询时重建缓存。 */
	mutable bool bRefreshPlainSearchableText = true;
	/** 从描述富文本提取的可搜索纯文本缓存。 */
	mutable FString AutoGenerated_DescriptionPlainText;

	/** 是否参与分析上报；值类型设置默认开启。 */
	bool bReportAnalytics = false;

private:
	/** 是否已完成初始化；就绪前不应依赖设置值。 */
	bool bReady = false;

	/** 防止在宣布设置已更改时出现重入问题。 */
	bool bOnSettingChangedEventGuard = false;

	/** 防止在宣布设置的编辑条件已更改时出现重入问题。 */
	bool bOnEditConditionsChangedEventGuard = false;

	bool bAdjustListViewPostRefresh = true;

	/** 缓存最近一次计算的可编辑状态，避免每次查询重复求值。 */
	FGameSettingEditableState EditableStateCache;
};

#undef UE_API
