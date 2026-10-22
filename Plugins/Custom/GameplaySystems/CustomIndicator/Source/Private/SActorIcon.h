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

/** 计算指示器投影和边缘箭头布局，并管理异步加载的控件池。 */
class SActorIcon : public SPanel, public FAsyncLoadMixin, public FGCObject
{
public:
	/** 指示器面板使用的槽位。 */
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

		/** 由 SActorIcon::AddReferencedObjects 通过 AllIndicators 保持引用。 */
		UIndicatorDescriptorDataObj* Indicator;
		FVector2D ScreenPosition;
		double Depth;
		int32 Priority;

		uint8 bIsIndicatorVisible : 1;
		uint8 bInFrontOfCamera : 1;
		uint8 bHasValidScreenPosition : 1;
		uint8 bDirty : 1;

		/** 布局阶段缓存的屏幕边缘限制状态，在后续更新中检测变化。 */
		mutable uint8 bWasIndicatorClamped : 1;
		mutable uint8 bWasIndicatorClampedStatusChanged : 1;

		friend class SActorIcon;
	};

	/** 指示器面板使用的槽位。 */
	class FArrowSlot : public TSlotBase<FArrowSlot>
	{
	};

	// Slate 构造参数。
	SLATE_BEGIN_ARGS(SActorIcon)
		{
			_Visibility = EVisibility::HitTestInvisible;
		}

		/** 构造时可配置的指示器槽位。 */
		SLATE_SLOT_ARGUMENT(SActorIcon::FSlot, Slots)

		// 结束 Slate 参数声明。
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

	/** 根据控件期望尺寸与对齐方式，计算位置偏移及屏幕边界留白。 */
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

	/** 画布中的指示器槽位。 */
	TPanelChildren<FSlot> CanvasChildren;
	mutable TPanelChildren<FArrowSlot> ArrowChildren;
	FCombinedChildren AllChildren;

	FUserWidgetPool IndicatorPool;

	const FSlateBrush* ActorCanvasArrowBrush = nullptr;

	mutable int32 NextArrowIndex = 0;
	mutable int32 ArrowIndexLastUpdate = 0;

	/** 是否为依次绘制的控件累进绘制层级；启用后可能减少批处理机会。 */
	bool bDrawElementsInOrder = false;

	bool bShowAnyIndicators = false;

	mutable TOptional<FGeometry> OptionalPaintGeometry;

	TSharedPtr<FActiveTimerHandle> TickHandle;
};
