// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSettingValueScalar.h"

#include "GameSettingValueScalarDynamic.generated.h"

#define UE_API GAMESETTINGS_API

struct FNumberFormattingOptions;

class FGameSettingDataSource;
class UObject;

//////////////////////////////////////////////////////////////////////////
// UGameSettingValueScalarDynamic
//////////////////////////////////////////////////////////////////////////

typedef TFunction<FText(double SourceValue, double NormalizedValue)> FSettingScalarFormatFunction;

UCLASS(MinimalAPI)
class UGameSettingValueScalarDynamic : public UGameSettingValueScalar
{
	GENERATED_BODY()

public:
	static UE_API FSettingScalarFormatFunction Raw;
	static UE_API FSettingScalarFormatFunction RawOneDecimal;
	static UE_API FSettingScalarFormatFunction RawTwoDecimals;
	static UE_API FSettingScalarFormatFunction ZeroToOnePercent;
	static UE_API FSettingScalarFormatFunction ZeroToOnePercent_OneDecimal;
	static UE_API FSettingScalarFormatFunction SourceAsPercent1;
	static UE_API FSettingScalarFormatFunction SourceAsPercent100;
	static UE_API FSettingScalarFormatFunction SourceAsInteger;

private:
	static const FNumberFormattingOptions& GetOneDecimalFormattingOptions();

public:
	UE_API UGameSettingValueScalarDynamic();

	/** UGameSettingValue */
	UE_API virtual void Startup() override;
	UE_API virtual void StoreInitial() override;
	UE_API virtual void ResetToDefault() override;
	UE_API virtual void RestoreToInitial() override;

	/** UGameSettingValueScalar */
	UE_API virtual TOptional<double> GetDefaultValue() const override;
	UE_API virtual void SetValue(double Value, EGameSettingChangeReason Reason = EGameSettingChangeReason::Change) override;
	UE_API virtual double GetValue() const override;
	UE_API virtual TRange<double> GetSourceRange() const override;
	UE_API virtual double GetSourceStep() const override;
	UE_API virtual FText GetFormattedText() const override;

	/** UGameSettingValueDiscreteDynamic */
	UE_API void SetDynamicGetter(const TSharedRef<FGameSettingDataSource>& InGetter);
	UE_API void SetDynamicSetter(const TSharedRef<FGameSettingDataSource>& InSetter);
	UE_API void SetDefaultValue(double InValue);

	/**  */
	UE_API void SetDisplayFormat(FSettingScalarFormatFunction InDisplayFormat);

	/**  */
	UE_API void SetSourceRangeAndStep(const TRange<double>& InRange, double InSourceStep);

	/**
	 * SetSourceRangeAndStep 定义了数值可以移动的实际范围，但通常
	 * 用户真正的最小值大于源范围的最小值，例如，某个滑块的范围可能是 0..100，
	 * 但你希望限制该滑块，使其在显示从 0 到 100 的进度条的同时，
	 * 用户不能将值设置得低于某个最小值，例如 1。
	 * 这就是最小值限制。
	 */
	UE_API void SetMinimumLimit(const TOptional<double>& InMinimum);

	/**
	 * SetSourceRangeAndStep 定义了数值可以移动的实际范围，但很少情况下
	 * 用户真正的最大值小于源范围的最大值，例如，某个滑块的范围可能是 0..100，
	 * 但你希望限制该滑块，使其在显示从 0 到 100 的进度条的同时，
	 * 用户不能将值设置得高于某个最大值，例如 95。
	 * 这就是最大值限制。
	 */
	UE_API void SetMaximumLimit(const TOptional<double>& InMaximum);

protected:
	/** UGameSettingValue */
	UE_API virtual void OnInitialized() override;

	UE_API void OnDataSourcesReady();

protected:
	TSharedPtr<FGameSettingDataSource> Getter;
	TSharedPtr<FGameSettingDataSource> Setter;

	TOptional<double> DefaultValue;
	double InitialValue = 0;

	TRange<double> SourceRange = TRange<double>(0, 1);
	double SourceStep = 0.01;
	TOptional<double> Minimum;
	TOptional<double> Maximum;

	FSettingScalarFormatFunction DisplayFormat;
};

#undef UE_API
