// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AsyncLoadMixin.h"
#include "Blueprint/UserWidgetPool.h"
#include "Widgets/SPanel.h"

class FActiveTimerHandle;
class FArrangedChildren;
class FChildren;
class FPaintArgs;
class FReferenceCollector;
class FSlateRect;
class FSlateWindowElementList;
class FWidgetStyle;
class UIndicatorDescriptorDataObj;
class UIndicatorManagerComponent;
struct FSlateBrush;

/*
 * 实质上作为指示器的计算和渲染的控件
 */
class SActorIcon : public SPanel, public FAsyncLoadMixin, public FGCObject
{
public:
	/** ActorIcon 专用槽类 */
	class FSlot : public TSlotBase<FSlot>
	{
	public:
		FSlot(UIndicatorDescriptorDataObj* InIndicator)
			: TSlotBase<FSlot>()
			  , Indicator(InIndicator)
			  , ScreenPosition(FVector2D::ZeroVector)
			  , Depth(0)
			  , Priority(0.f)
			  , bIsIndicatorVisible(true)
			  , bInFrontOfCamera(true)
			  , bHasValidScreenPosition(false)
			  , bDirty(true)
			  , bWasIndicatorClamped(false)
			  , bWasIndicatorClampedStatusChanged(false)
		{
		}

		SLATE_SLOT_BEGIN_ARGS(FSlot, TSlotBase<FSlot>)
		SLATE_SLOT_END_ARGS()

		using TSlotBase<FSlot>::Construct;

		bool GetIsIndicatorVisible() const { return bIsIndicatorVisible; }

		void SetIsIndicatorVisible(bool bVisible)
		{
			if (bIsIndicatorVisible != bVisible)
			{
				bIsIndicatorVisible = bVisible;
				bDirty = true;
			}

			RefreshVisibility();
		}

		FVector2D GetScreenPosition() const { return ScreenPosition; }

		void SetScreenPosition(FVector2D InScreenPosition)
		{
			if (ScreenPosition != InScreenPosition)
			{
				ScreenPosition = InScreenPosition;
				bDirty = true;
			}
		}

		double GetDepth() const { return Depth; }

		void SetDepth(double InDepth)
		{
			if (Depth != InDepth)
			{
				Depth = InDepth;
				bDirty = true;
			}
		}

		int32 GetPriority() const { return Priority; }

		void SetPriority(int32 InPriority)
		{
			if (Priority != InPriority)
			{
				Priority = InPriority;
				bDirty = true;
			}
		}

		bool GetInFrontOfCamera() const { return bInFrontOfCamera; }

		void SetInFrontOfCamera(bool bInFront)
		{
			if (bInFrontOfCamera != bInFront)
			{
				bInFrontOfCamera = bInFront;
				bDirty = true;
			}

			RefreshVisibility();
		}

		bool HasValidScreenPosition() const { return bHasValidScreenPosition; }

		void SetHasValidScreenPosition(bool bValidScreenPosition)
		{
			if (bHasValidScreenPosition != bValidScreenPosition)
			{
				bHasValidScreenPosition = bValidScreenPosition;
				bDirty = true;
			}

			RefreshVisibility();
		}

		bool bIsDirty() const { return bDirty; }

		void ClearDirtyFlag()
		{
			bDirty = false;
		}

		bool WasIndicatorClamped() const { return bWasIndicatorClamped; }

		void SetWasIndicatorClamped(bool bWasClamped) const
		{
			if (bWasClamped != bWasIndicatorClamped)
			{
				bWasIndicatorClamped = bWasClamped;
				bWasIndicatorClampedStatusChanged = true;
			}
		}

		bool WasIndicatorClampedStatusChanged() const { return bWasIndicatorClampedStatusChanged; }

		void ClearIndicatorClampedStatusChangedFlag()
		{
			bWasIndicatorClampedStatusChanged = false;
		}

	private:
		void RefreshVisibility()
		{
			const bool bIsVisible = bIsIndicatorVisible && bHasValidScreenPosition;
			GetWidget()->SetVisibility(bIsVisible ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed);
		}

		//由 SActorCanvas：：AddReferencedObjects维持生命
		UIndicatorDescriptorDataObj* Indicator;
		FVector2D ScreenPosition;
		double Depth;
		int32 Priority;

		uint8 bIsIndicatorVisible : 1;
		uint8 bInFrontOfCamera : 1;
		uint8 bHasValidScreenPosition : 1;
		uint8 bDirty : 1;

		/** 
		 缓存和帧延迟值，显示指示器是否在上一帧被视觉屏蔽;半黑客式的可变实现，作为在const绘画操作中缓存的
		 */
		mutable uint8 bWasIndicatorClamped : 1;
		mutable uint8 bWasIndicatorClampedStatusChanged : 1;

		friend class SActorIcon;
	};

	/** ActorIcon 专用槽类 */
	class FArrowSlot : public TSlotBase<FArrowSlot>
	{
	};

	/** 开始定义此 Slate 小部件的参数 */
	SLATE_BEGIN_ARGS(SActorIcon)
		{
			_Visibility = EVisibility::HitTestInvisible;
		}

		/** 表示我们有一个槽位，这个小部件支持它 */
		SLATE_SLOT_ARGUMENT(SActorIcon::FSlot, Slots)

		/** 这总是在最后 */
	SLATE_END_ARGS()

	SActorIcon()
		: CanvasChildren(this)
		  , ArrowChildren(this)
		  , AllChildren(this)
	{
		AllChildren.AddChildren(CanvasChildren);
		AllChildren.AddChildren(ArrowChildren);
	}

	void Construct(const FArguments& InArgs, const FLocalPlayerContext& InCtx, const FSlateBrush* ActorCanvasArrowBrush);

	// SWidget Interface
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D::ZeroVector; }
	virtual FChildren* GetChildren() override { return &AllChildren; }
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const;
	// End SWidget

	void SetDrawElementsInOrder(bool bInDrawElementsInOrder) { bDrawElementsInOrder = bInDrawElementsInOrder; }

	virtual FString GetReferencerName() const override;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

private:
	void OnIndicatorAdded(UIndicatorDescriptorDataObj* Indicator);
	void OnIndicatorRemoved(UIndicatorDescriptorDataObj* Indicator);

	void AddIndicatorForEntry(UIndicatorDescriptorDataObj* Indicator);
	void RemoveIndicatorForEntry(UIndicatorDescriptorDataObj* Indicator);

	using FScopedWidgetSlotArguments = TPanelChildren<FSlot>::FScopedWidgetSlotArguments;
	FScopedWidgetSlotArguments AddActorSlot(UIndicatorDescriptorDataObj* Indicator);
	int32 RemoveActorSlot(const TSharedRef<SWidget>& SlotWidget);

	void SetShowAnyIndicators(bool bIndicators);
	EActiveTimerReturnType UpdateCanvas(double InCurrentTime, float InDeltaTime);

	/** 用于计算偏移的辅助函数 */
	void GetOffsetAndSize(const UIndicatorDescriptorDataObj* Indicator,
	                      FVector2D& OutSize,
	                      FVector2D& OutOffset,
	                      FVector2D& OutPaddingMin,
	                      FVector2D& OutPaddingMax) const;

	void UpdateActiveTimer();

private:
	TArray<TObjectPtr<UIndicatorDescriptorDataObj>> AllIndicators;
	TArray<UIndicatorDescriptorDataObj*> InactiveIndicators;

	FLocalPlayerContext LocalPlayerContext;
	TWeakObjectPtr<UIndicatorManagerComponent> IndicatorComponentPtr;

	/** 这幅画布上的所有槽位 */
	TPanelChildren<FSlot> CanvasChildren;
	mutable TPanelChildren<FArrowSlot> ArrowChildren;
	FCombinedChildren AllChildren;

	FUserWidgetPool IndicatorPool;

	const FSlateBrush* ActorCanvasArrowBrush = nullptr;

	mutable int32 NextArrowIndex = 0;
	mutable int32 ArrowIndexLastUpdate = 0;

	/** 是否按添加元素到画布的顺序绘制。注意：启用此功能会禁用批处理，并导致更多抽取调用 */
	bool bDrawElementsInOrder = false;

	bool bShowAnyIndicators = false;

	mutable TOptional<FGeometry> OptionalPaintGeometry;

	TSharedPtr<FActiveTimerHandle> TickHandle;
};
