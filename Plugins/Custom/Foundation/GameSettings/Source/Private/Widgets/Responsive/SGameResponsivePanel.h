// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/SCompoundWidget.h"

class FArrangedChildren;
class SWidget;
struct FGeometry;

class SGameResponsivePanel : public SCompoundWidget
{
public:
	typedef SGridPanel::FSlot FSlot;

public:
	SLATE_BEGIN_ARGS(SGameResponsivePanel)
		{
			_Visibility = EVisibility::SelfHitTestInvisible;
		}

	SLATE_END_ARGS()

public:
	SGameResponsivePanel();

	/** 根据声明参数构造控件。 */
	void Construct(const FArguments& InArgs);

	/** 添加内容槽，并返回新槽。 */
	FSlot& AddSlot();

	/** 移除 SlotWidget 所在的内容槽。 */
	int32 RemoveSlot(const TSharedRef<SWidget>& SlotWidget);

	/** 从面板移除全部内容槽。 */
	void ClearChildren();

	void EnableVerticalStacking(const bool bCanVerticallyWrap);

protected:
	// 开始 SWidget 覆写。
	virtual bool CustomPrepass(float LayoutScaleMultiplier) override;
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual float GetRelativeLayoutScale(int32 ChildIndex, float LayoutScaleMultiplier) const override;
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const;
	// 结束 SWidget 覆写。

	bool ShouldWrap() const;

	void RefreshResponsiveness();
	void RefreshLayout();

protected:
	TSharedRef<SGridPanel> InnerGrid;
	TArray<SGridPanel::FSlot*> InnerSlots;

	FVector2D PhysialScreenSize = FVector2D(0, 0);
	float Scale = 1;

	uint8 bCanWrapVertically : 1;
};
