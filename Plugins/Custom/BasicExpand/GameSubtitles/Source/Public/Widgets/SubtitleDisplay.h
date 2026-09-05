// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/Widget.h"
#include "Styling/SlateTypes.h"
#include "SubtitleDisplaySubsystem.h"

#include "SubtitleDisplay.generated.h"

#define UE_API GAMESUBTITLES_API

class USubtitleDisplayOptions;

struct FSubtitleFormat;

UCLASS(MinimalAPI, BlueprintType, Blueprintable, meta = (DisableNativeTick))
class USubtitleDisplay : public UWidget
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Display Info")
	FSubtitleFormat Format;

	UPROPERTY(EditAnywhere, Category = "Display Info")
	TObjectPtr<USubtitleDisplayOptions> Options;

	// 当文本长度超过此宽度时是否换行显示；如果该值为零或负数，则不换行。
	UPROPERTY(EditAnywhere, Category="Display Info")
	float WrapTextAt;

	UFUNCTION(BlueprintCallable, Category = Subtitles, Meta = (Tooltip = "True if there are subtitles currently.  False if the subtitle text is empty."))
	UE_API bool HasSubtitles() const;

	/** 设计该控件时显示的预览文本 */
	UPROPERTY(EditAnywhere, Category="Preview")
	bool bPreviewMode;

	/** 设计该控件时显示的预览文本 */
	UPROPERTY(EditAnywhere, Category="Preview")
	FText PreviewText;

public:
	// UWidget Public Interface
	UE_API virtual void SynchronizeProperties() override;
	UE_API virtual void ReleaseSlateResources(bool bReleaseChildren) override;
#if WITH_EDITOR
	UE_API virtual void ValidateCompiledDefaults(class IWidgetCompilerLog& CompileLog) const;
#endif
	// End UWidget Public Interface

protected:
	// UWidget Protected Interface
	UE_API virtual TSharedRef<class SWidget> RebuildWidget() override;
	// End UWidget Protected Interface

	UE_API void HandleSubtitleDisplayOptionsChanged(const FSubtitleFormat& InDisplayFormat);

private:
	void RebuildStyle();

private:
	UPROPERTY(Transient)
	FTextBlockStyle GeneratedStyle;

	UPROPERTY(Transient)
	FSlateBrush GeneratedBackgroundBorder;

	/** 实际用于显示字幕数据的控件 */
	TSharedPtr<class SSubtitleDisplay> SubtitleWidget;
};

#undef UE_API
