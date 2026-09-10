// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "Styling/SlateWidgetStyleAsset.h"
#include "Styling/SlateWidgetStyleContainerBase.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Accessibility/SlateWidgetAccessibleTypes.h"

#define UE_API GAMESUBTITLES_API

class FText;
struct FSlateBrush;

/**
 * 用于在视口的某个位置显示字幕的控件
 */
class SSubtitleDisplay : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSubtitleDisplay)
			: _TextStyle(&FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText"))
			  , _WrapTextAt(0.f)
			  , _ManualSubtitles(false)
		{
		}

		SLATE_STYLE_ARGUMENT(FTextBlockStyle, TextStyle)

		/** 当文本长度超过此宽度时是否换行显示；如果该值为零或负数，则不换行。 */
		SLATE_ATTRIBUTE(float, WrapTextAt)

		SLATE_ATTRIBUTE(bool, ManualSubtitles)

	SLATE_END_ARGS()

	UE_API ~SSubtitleDisplay();

	UE_API void Construct(const FArguments& InArgs);

	UE_API void SetTextStyle(const FTextBlockStyle& InTextStyle);

	UE_API void SetBackgroundBrush(const FSlateBrush* InSlateBrush);

	UE_API void SetCurrentSubtitleText(const FText& SubtitleText);

	UE_API bool HasSubtitles() const;

	/** 参见 WrapTextAt 属性 */
	UE_API void SetWrapTextAt(const TAttribute<float>& InWrapTextAt);

private:
	void HandleSubtitleChanged(const FText& SubtitleText);

private:
	TSharedPtr<class SBorder> Background;

	/** 实际用于显示字幕文本的控件 */
	TSharedPtr<class SRichTextBlock> TextDisplay;
};

#undef UE_API
