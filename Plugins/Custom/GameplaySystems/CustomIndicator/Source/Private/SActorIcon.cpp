// Copyright Epic Games, Inc. All Rights Reserved.

#include "SActorIcon.h"

#include "Engine/GameViewportClient.h"
#include "IndicatorWidgetInterface.h"
#include "Layout/ArrangedChildren.h"
#include "IndicatorManagerComponent.h"
#include "SceneView.h"
#include "IndicatorDescriptorDataObj.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SLeafWidget.h"

class FSlateRect;

namespace EArrowDirection
{
	enum Type
	{
		Left,
		Top,
		Right,
		Bottom,
		MAX
	};
}

/** 四个边缘方向对应的箭头旋转角度。 */
const float ArrowRotations[EArrowDirection::MAX] =
{
	270.0f,
	0.0f,
	90.0f,
	180.0f
};

/** 四个边缘方向对应的箭头偏移方向。 */
const FVector2D ArrowOffsets[EArrowDirection::MAX] =
{
	FVector2D(-1.0f, 0.0f),
	FVector2D(0.0f, -1.0f),
	FVector2D(1.0f, 0.0f),
	FVector2D(0.0f, 1.0f)
};


class SActorIconArrowWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SActorIconArrowWidget)
		{
		}

		// 结束 Slate 参数声明。
	SLATE_END_ARGS()

	/** 初始化箭头旋转和画刷引用。 */
	SActorIconArrowWidget()
		: Rotation(0.0f)
		  , Arrow(nullptr)
	{
	}

	/** 绑定箭头画刷，并禁用控件自身的 Tick。 */
	void Construct(const FArguments& InArgs, const FSlateBrush* ActorCanvasArrowBrush)
	{
		Arrow = ActorCanvasArrowBrush;
		SetCanTick(false);
	}

	virtual int32 OnPaint(const FPaintArgs& Args,
	                      const FGeometry& AllottedGeometry,
	                      const FSlateRect& MyClippingRect,
	                      FSlateWindowElementList& OutDrawElements,
	                      int32 LayerId,
	                      const FWidgetStyle& InWidgetStyle,
	                      bool bParentEnabled) const override
	{
		int32 MaxLayerId = LayerId;

		if (Arrow)
		{
			const bool bIsEnabled = ShouldBeEnabled(bParentEnabled);
			const ESlateDrawEffect DrawEffects = bIsEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
			const FColor FinalColorAndOpacity = (InWidgetStyle.GetColorAndOpacityTint() * Arrow->GetTint(InWidgetStyle)).ToFColor(true);

			FSlateDrawElement::MakeRotatedBox(
				OutDrawElements,
				MaxLayerId++,
				AllottedGeometry.ToPaintGeometry(Arrow->ImageSize, FSlateLayoutTransform()),
				Arrow,
				DrawEffects,
				FMath::DegreesToRadians(GetRotation()),
				TOptional<FVector2D>(),
				FSlateDrawElement::RelativeToElement,
				FinalColorAndOpacity
			);
		}

		return MaxLayerId;
	}

	FORCEINLINE void SetRotation(float InRotation)
	{
		Rotation = FMath::Fmod(InRotation, 360.0f);
	}

	FORCEINLINE float GetRotation() const
	{
		return Rotation;
	}

	virtual FVector2D ComputeDesiredSize(float) const override
	{
		if (Arrow)
		{
			return Arrow->ImageSize;
		}
		else
		{
			return FVector2D::ZeroVector;
		}
	}

private:
	float Rotation;

	const FSlateBrush* Arrow;
};

void SActorIcon::Construct(const FArguments& InArgs, const FLocalPlayerContext& InLocalPlayerContext, const FSlateBrush* InActorCanvasArrowBrush)
{
	LocalPlayerContext = InLocalPlayerContext;
	ActorCanvasArrowBrush = InActorCanvasArrowBrush;

	IndicatorPool.SetWorld(LocalPlayerContext.GetWorld());

	SetCanTick(false);
	SetVisibility(EVisibility::SelfHitTestInvisible);

	// 预先创建十个可复用的边缘箭头，默认隐藏。
	for (int32 i = 0; i < 10; ++i)
	{
		TSharedRef<SActorIconArrowWidget> ArrowWidget = SNew(SActorIconArrowWidget, ActorCanvasArrowBrush);
		ArrowWidget->SetVisibility(EVisibility::Collapsed);

		ArrowChildren.AddSlot(MoveTemp(
			FArrowSlot::FSlotArguments(MakeUnique<FArrowSlot>())
			[
				ArrowWidget
			]
		));
	}

	UpdateActiveTimer();
}

EActiveTimerReturnType SActorIcon::UpdateCanvas(double InCurrentTime, float InDeltaTime)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SActorIcon_UpdateCanvas);

	if (!OptionalPaintGeometry.IsSet())
	{
		return EActiveTimerReturnType::Continue;
	}

	// 使用所属本地玩家的视角和指示器管理组件。
	ULocalPlayer* LocalPlayer = LocalPlayerContext.GetLocalPlayer();
	UIndicatorManagerComponent* IndicatorComponent = IndicatorComponentPtr.Get();
	if (IndicatorComponent == nullptr)
	{
		IndicatorComponent = UIndicatorManagerComponent::GetComponent(LocalPlayerContext.GetPlayerController());
		if (IndicatorComponent)
		{
			// 管理组件重新可用时同步 World，避免控件池沿用旧世界。
			IndicatorPool.SetWorld(LocalPlayerContext.GetWorld());

			IndicatorComponentPtr = IndicatorComponent;
			IndicatorComponent->OnIndicatorAdded.AddSP(this, &SActorIcon::OnIndicatorAdded);
			IndicatorComponent->OnIndicatorRemoved.AddSP(this, &SActorIcon::OnIndicatorRemoved);
			for (UIndicatorDescriptorDataObj* Indicator : IndicatorComponent->GetIndicators())
			{
				OnIndicatorAdded(Indicator);
			}
		}
		else
		{
			//TODO 隐藏一切
			return EActiveTimerReturnType::Continue;
		}
	}

	// 只有可用的本地玩家投影上下文才能显示指示器。
	if (LocalPlayer)
	{
		const FGeometry PaintGeometry = OptionalPaintGeometry.GetValue();

		FSceneViewProjectionData ProjectionData;
		if (LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, /*out*/ ProjectionData))
		{
			SetShowAnyIndicators(true);

			bool IndicatorsChanged = false;

			for (int32 ChildIndex = 0; ChildIndex < CanvasChildren.Num(); ++ChildIndex)
			{
				SActorIcon::FSlot& CurChild = CanvasChildren[ChildIndex];
				UIndicatorDescriptorDataObj* Indicator = CurChild.Indicator;

				// 关联组件失效且允许自动注销时，完整移除该指示器。
				if (Indicator->CanAutomaticallyRemove())
				{
					IndicatorsChanged = true;

					// 经管理器事件清理控件、槽位及持有引用，描述对象随后可由 GC 回收。
					Indicator->UnregisterIndicator();
					// 当前槽位已移除，回退索引以免跳过后移的条目。
					--ChildIndex;
					continue;
				}

				CurChild.SetIsIndicatorVisible(Indicator->GetIsVisible());

				// 使用距离平方剔除超距指示器，后续跳过投影与绘制。
				const float MaxDrawDistance = Indicator->IndicatorDescriptorParameter.MaxDrawDistance;
				if (MaxDrawDistance > 0.f && Indicator->IndicatorDescriptorParameter.SceneComponent)
				{
					const float DistanceSq = FVector::DistSquared(ProjectionData.ViewOrigin, Indicator->IndicatorDescriptorParameter.SceneComponent->GetComponentLocation());
					CurChild.SetIsIndicatorVisible(DistanceSq <= MaxDrawDistance * MaxDrawDistance);
				}

				if (!CurChild.GetIsIndicatorVisible())
				{
					IndicatorsChanged |= CurChild.bIsDirty();
					CurChild.ClearDirtyFlag();
					continue;
				}

				// 边缘限制状态变化后请求重绘；通知回调当前仍被注释。
				if (CurChild.WasIndicatorClampedStatusChanged())
				{
					//Indicator->OnIndicatorClampedStatusChanged(CurChild.WasIndicatorClamped());
					CurChild.ClearIndicatorClampedStatusChangedFlag();
					IndicatorsChanged = true;
				}

				FVector ScreenPositionWithDepth;

				const bool Success = UIndicatorDescriptorDataObj::Project(*Indicator, ProjectionData, PaintGeometry.Size, OUT ScreenPositionWithDepth);

				if (!Success)
				{
					CurChild.SetHasValidScreenPosition(false);
					CurChild.SetInFrontOfCamera(false);

					IndicatorsChanged |= CurChild.bIsDirty();
					CurChild.ClearDirtyFlag();
					continue;
				}

				CurChild.SetInFrontOfCamera(Success);
				CurChild.SetHasValidScreenPosition(CurChild.GetInFrontOfCamera() || Indicator->IndicatorDescriptorParameter.bClampToScreen);

				if (CurChild.HasValidScreenPosition())
				{
					// 仅在位置有效时更新投影缓存，避免无效位置触发重绘。
					CurChild.SetScreenPosition(FVector2D(ScreenPositionWithDepth));
					CurChild.SetDepth(ScreenPositionWithDepth.Z); // Z 为相机距离，同优先级时近处指示器后绘制。
				}

				CurChild.SetPriority(Indicator->IndicatorDescriptorParameter.Priority);

				IndicatorsChanged |= CurChild.bIsDirty();
				CurChild.ClearDirtyFlag();
			}

			if (IndicatorsChanged)
			{
				Invalidate(EInvalidateWidget::Paint);
			}
		}
		else
		{
			SetShowAnyIndicators(false);
		}
	}
	else
	{
		SetShowAnyIndicators(false);
	}

	if (AllIndicators.Num() == 0)
	{
		TickHandle.Reset();
		return EActiveTimerReturnType::Stop;
	}
	else
	{
		return EActiveTimerReturnType::Continue;
	}
}

void SActorIcon::SetShowAnyIndicators(bool bIndicators)
{
	if (bShowAnyIndicators != bIndicators)
	{
		bShowAnyIndicators = bIndicators;

		if (!bShowAnyIndicators)
		{
			for (int32 ChildIndex = 0; ChildIndex < AllChildren.Num(); ChildIndex++)
			{
				AllChildren.GetChildAt(ChildIndex)->SetVisibility(EVisibility::Collapsed);
			}
		}
	}
}

void SActorIcon::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SActorIcon_OnArrangeChildren);

	NextArrowIndex = 0;

	// 只有可用的本地玩家投影上下文才能显示指示器。
	if (bShowAnyIndicators)
	{
		const FVector2D ArrowWidgetSize = ActorCanvasArrowBrush->GetImageSize();
		const FIntPoint FixedPadding = FIntPoint(10.0f, 10.0f) + FIntPoint(ArrowWidgetSize.X, ArrowWidgetSize.Y);
		const FVector Center = FVector(AllottedGeometry.Size * 0.5f, 0.0f);

		// 先按优先级排序，同优先级按距离由远到近绘制。
		TArray<const SActorIcon::FSlot*> SortedSlots;
		for (int32 ChildIndex = 0; ChildIndex < CanvasChildren.Num(); ++ChildIndex)
		{
			SortedSlots.Add(&CanvasChildren[ChildIndex]);
		}

		SortedSlots.StableSort([](const SActorIcon::FSlot& A, const SActorIcon::FSlot& B)
		{
			return A.GetPriority() == B.GetPriority() ? A.GetDepth() > B.GetDepth() : A.GetPriority() < B.GetPriority();
		});

		// 按排序结果依次安排指示器。
		for (int32 ChildIndex = 0; ChildIndex < SortedSlots.Num(); ++ChildIndex)
		{
			// 读取当前槽位及其描述对象。
			const SActorIcon::FSlot& CurChild = *SortedSlots[ChildIndex];
			const UIndicatorDescriptorDataObj* Indicator = CurChild.Indicator;

			// 跳过当前布局不接受的可见性状态。
			if (!ArrangedChildren.Accepts(CurChild.GetWidget()->GetVisibility()))
			{
				CurChild.SetWasIndicatorClamped(false);
				continue;
			}

			FVector2D ScreenPosition = CurChild.GetScreenPosition();
			const bool bInFrontOfCamera = CurChild.GetInFrontOfCamera();
			const FVector2D UnclampedScreenPosition = ScreenPosition; // 保留限制前的位置，供连续方向箭头计算使用。

			// 是否将指示器限制到屏幕边缘由描述配置决定。
			const bool bShouldClamp = Indicator->IndicatorDescriptorParameter.bClampToScreen;

			// 根据控件尺寸和对齐方式预留屏幕边界空间。
			FVector2D SlotSize, SlotOffset, SlotPaddingMin, SlotPaddingMax;
			GetOffsetAndSize(Indicator, SlotSize, SlotOffset, SlotPaddingMin, SlotPaddingMax);

			bool bWasIndicatorClamped = false;

			// 仅为启用边缘限制的指示器计算边界交点。
			if (bShouldClamp)
			{
				// 记录指示器最终落在哪条屏幕边缘上。
				EArrowDirection::Type ClampDir = EArrowDirection::MAX;

				// 扣除控件和箭头所需留白，得到可放置区域。
				const FIntPoint RectMin = FIntPoint(SlotPaddingMin.X, SlotPaddingMin.Y) + FixedPadding;
				const FIntPoint RectMax = FIntPoint(AllottedGeometry.Size.X - SlotPaddingMax.X, AllottedGeometry.Size.Y - SlotPaddingMax.Y) - FixedPadding;
				const FIntRect ClampRect(RectMin, RectMax);

				// 屏幕位置超出区域时，求中心到目标线段与边界的交点。
				if (!ClampRect.Contains(FIntPoint(ScreenPosition.X, ScreenPosition.Y)))
				{
					const FPlane Planes[] =
					{
						FPlane(FVector(1.0f, 0.0f, 0.0f), ClampRect.Min.X), // 左边界。
						FPlane(FVector(0.0f, 1.0f, 0.0f), ClampRect.Min.Y), // 上边界。
						FPlane(FVector(-1.0f, 0.0f, 0.0f), -ClampRect.Max.X), // 右边界。
						FPlane(FVector(0.0f, -1.0f, 0.0f), -ClampRect.Max.Y) // 下边界。
					};

					for (int32 i = 0; i < EArrowDirection::MAX; ++i)
					{
						FVector NewPoint;
						if (FMath::SegmentPlaneIntersection(Center, FVector(ScreenPosition, 0.0f), Planes[i], NewPoint))
						{
							ClampDir = (EArrowDirection::Type)i;
							ScreenPosition = FVector2D(NewPoint);
						}
					}
				}
				else if (!bInFrontOfCamera)
				{
					const float ScreenXNorm = ScreenPosition.X / (RectMax.X - RectMin.X);
					const float ScreenYNorm = ScreenPosition.Y / (RectMax.Y - RectMin.Y);
					// 相机后方的目标也需要固定到屏幕边缘。
					if (ScreenXNorm < ScreenYNorm)
					{
						if (ScreenXNorm < (-ScreenYNorm + 1.0f))
						{
							ClampDir = EArrowDirection::Left;
							ScreenPosition.X = ClampRect.Min.X;
						}
						else
						{
							ClampDir = EArrowDirection::Bottom;
							ScreenPosition.Y = ClampRect.Max.Y;
						}
					}
					else
					{
						if (ScreenXNorm < (-ScreenYNorm + 1.0f))
						{
							ClampDir = EArrowDirection::Top;
							ScreenPosition.Y = ClampRect.Min.Y;
						}
						else
						{
							ClampDir = EArrowDirection::Right;
							ScreenPosition.X = ClampRect.Max.X;
						}
					}
				}

				bWasIndicatorClamped = (ClampDir != EArrowDirection::MAX);

				// 仅在发生边缘限制且有可用箭头槽位时显示箭头。
				if (Indicator->IndicatorDescriptorParameter.bShowClampToScreenArrow &&
					bWasIndicatorClamped &&
					ArrowChildren.IsValidIndex(NextArrowIndex))
				{
					// 默认使用四个固定方向；连续模式按边缘位置指向原始目标位置。
					FVector2D ArrowOffsetDirection = ArrowOffsets[ClampDir];
					float ArrowRotation = ArrowRotations[ClampDir];
					if (Indicator->IndicatorDescriptorParameter.bUse360ClampArrow)
					{
						const FVector2D ArrowDirection = (UnclampedScreenPosition - ScreenPosition).GetSafeNormal();
						if (!ArrowDirection.IsNearlyZero())
						{
							ArrowOffsetDirection = ArrowDirection;
							ArrowRotation = FMath::RadiansToDegrees(FMath::Atan2(ArrowDirection.X, -ArrowDirection.Y));
							if (ArrowRotation < 0.0f) { ArrowRotation += 360.0f; }
						}
					}

					// 复用下一个预建的箭头控件。
					TSharedRef<SActorIconArrowWidget> ArrowWidgetToUse = StaticCastSharedRef<SActorIconArrowWidget>(ArrowChildren.GetChildAt(NextArrowIndex));
					NextArrowIndex++;

					// 应用当前目标对应的箭头角度。
					ArrowWidgetToUse->SetRotation(ArrowRotation);

					// 按控件与箭头的尺寸计算相互间距。
					const FVector2D OffsetMagnitude = (SlotSize + ArrowWidgetSize) * 0.5f;

					// 将箭头中心对齐到计算位置。
					const FVector2D ArrowCenteringOffset = -(ArrowWidgetSize * 0.5f);

					FVector2D ArrowAlignmentOffset = FVector2D::ZeroVector;
					switch (Indicator->IndicatorDescriptorParameter.VAlignment)
					{
					case VAlign_Top:
						ArrowAlignmentOffset = SlotSize * FVector2D(0.0f, 0.5f);
						break;
					case VAlign_Bottom:
						ArrowAlignmentOffset = SlotSize * FVector2D(0.0f, -0.5f);
						break;
					}

					// 沿箭头方向偏移，避免遮住指示器。
					const FVector2D WidgetOffset = (OffsetMagnitude * ArrowOffsetDirection);

					const FVector2D FinalOffset = (WidgetOffset + ArrowAlignmentOffset + ArrowCenteringOffset);

					// 合并屏幕位置与布局偏移。
					const FVector2D FinalPosition = (ScreenPosition + FinalOffset);

					ArrowWidgetToUse->SetVisibility(EVisibility::HitTestInvisible);

					// 将箭头加入本次布局结果。
					ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(
						ArrowWidgetToUse, // 箭头控件。
						FinalPosition, // 相对父控件的位置。
						ArrowWidgetSize, // 箭头尺寸。
						1.f // 箭头缩放。
					));
				}
			}

			CurChild.SetWasIndicatorClamped(bWasIndicatorClamped);

			// 将指示器本体加入布局结果。
			ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(
				CurChild.GetWidget(),
				ScreenPosition + SlotOffset,
				SlotSize,
				1.f
			));
		}
	}

	if (NextArrowIndex < ArrowIndexLastUpdate)
	{
		for (int32 ArrowRemovedIndex = NextArrowIndex; ArrowRemovedIndex < ArrowIndexLastUpdate; ArrowRemovedIndex++)
		{
			ArrowChildren.GetChildAt(ArrowRemovedIndex)->SetVisibility(EVisibility::Collapsed);
		}
	}

	ArrowIndexLastUpdate = NextArrowIndex;
}

int32 SActorIcon::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SActorIcon_OnPaint);

	OptionalPaintGeometry = AllottedGeometry;

	FArrangedChildren ArrangedChildren(EVisibility::Visible);
	ArrangeChildren(AllottedGeometry, ArrangedChildren);

	int32 MaxLayerId = LayerId;

	const FPaintArgs NewArgs = Args.WithNewParent(this);
	const bool bShouldBeEnabled = ShouldBeEnabled(bParentEnabled);

	for (const FArrangedWidget& CurWidget : ArrangedChildren.GetInternalArray())
	{
		if (!IsChildWidgetCulled(MyCullingRect, CurWidget))
		{
			SWidget* MutableWidget = const_cast<SWidget*>(&CurWidget.Widget.Get());

			const int32 CurWidgetsMaxLayerId = CurWidget.Widget->Paint(NewArgs, CurWidget.Geometry, MyCullingRect, OutDrawElements, bDrawElementsInOrder ? MaxLayerId : LayerId, InWidgetStyle, bShouldBeEnabled);
			MaxLayerId = FMath::Max(MaxLayerId, CurWidgetsMaxLayerId);
		}
		else
		{
			//SlateGI - RemoveContent
		}
	}

	return MaxLayerId;
}

FString SActorIcon::GetReferencerName() const
{
	return TEXT("SActorIcon");
}

void SActorIcon::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObjects(AllIndicators);
}

void SActorIcon::OnIndicatorAdded(UIndicatorDescriptorDataObj* Indicator)
{
	AllIndicators.Add(Indicator);
	InactiveIndicators.Add(Indicator);

	AddIndicatorForEntry(Indicator);
}

void SActorIcon::OnIndicatorRemoved(UIndicatorDescriptorDataObj* Indicator)
{
	RemoveIndicatorForEntry(Indicator);

	AllIndicators.Remove(Indicator);
	InactiveIndicators.Remove(Indicator);
}

void SActorIcon::AddIndicatorForEntry(UIndicatorDescriptorDataObj* Indicator)
{
	// 异步加载控件类，并通过控件池复用显示实例。
	TSoftClassPtr<UUserWidget> IndicatorClass = Indicator->IndicatorDescriptorParameter.IndicatorWidgetClass;
	if (!IndicatorClass.IsNull())
	{
		TWeakObjectPtr<UIndicatorDescriptorDataObj> IndicatorPtr(Indicator);
		AsyncLoad(IndicatorClass, [this, IndicatorPtr, IndicatorClass]()
		{
			if (UIndicatorDescriptorDataObj* Indicator = IndicatorPtr.Get())
			{
				// 加载期间可能已注销；弱引用有效也必须确认仍由面板持有。
				if (!AllIndicators.Contains(Indicator))
				{
					return;
				}

				// 从控件池获取实例后，绑定描述对象并加入画布。
				if (UUserWidget* IndicatorWidget = IndicatorPool.GetOrCreateInstance(TSubclassOf<UUserWidget>(IndicatorClass.Get())))
				{
					if (IndicatorWidget->GetClass()->ImplementsInterface(UIndicatorWidgetInterface::StaticClass()))
					{
						IIndicatorWidgetInterface::Execute_BindIndicator(IndicatorWidget, Indicator);
					}

					Indicator->IndicatorWidget = IndicatorWidget;

					InactiveIndicators.Remove(Indicator);

					AddActorSlot(Indicator)
					[
						SAssignNew(Indicator->CanvasHost, SBox)
						[
							IndicatorWidget->TakeWidget()
						]
					];
				}
			}
		});
		StartAsyncLoading();
	}
}

void SActorIcon::RemoveIndicatorForEntry(UIndicatorDescriptorDataObj* Indicator)
{
	if (UUserWidget* IndicatorWidget = Indicator->IndicatorWidget.Get())
	{
		if (IndicatorWidget->GetClass()->ImplementsInterface(UIndicatorWidgetInterface::StaticClass()))
		{
			IIndicatorWidgetInterface::Execute_UnbindIndicator(IndicatorWidget, Indicator);
		}

		Indicator->IndicatorWidget = nullptr;

		IndicatorPool.Release(IndicatorWidget);
	}

	TSharedPtr<SWidget> CanvasHost = Indicator->CanvasHost.Pin();
	if (CanvasHost.IsValid())
	{
		RemoveActorSlot(CanvasHost.ToSharedRef());
		Indicator->CanvasHost.Reset();
	}
}

SActorIcon::FScopedWidgetSlotArguments SActorIcon::AddActorSlot(UIndicatorDescriptorDataObj* Indicator)
{
	TWeakPtr<SActorIcon> WeakCanvas = SharedThis(this);
	return FScopedWidgetSlotArguments{
		MakeUnique<FSlot>(Indicator), this->CanvasChildren, INDEX_NONE, [WeakCanvas](const FSlot*, int32)
		{
			if (TSharedPtr<SActorIcon> Canvas = WeakCanvas.Pin())
			{
				Canvas->UpdateActiveTimer();
			}
		}
	};
}

int32 SActorIcon::RemoveActorSlot(const TSharedRef<SWidget>& SlotWidget)
{
	for (int32 SlotIdx = 0; SlotIdx < CanvasChildren.Num(); ++SlotIdx)
	{
		if (SlotWidget == CanvasChildren[SlotIdx].GetWidget())
		{
			CanvasChildren.RemoveAt(SlotIdx);

			UpdateActiveTimer();

			return SlotIdx;
		}
	}

	return -1;
}

void SActorIcon::GetOffsetAndSize(const UIndicatorDescriptorDataObj* Indicator,
                                  FVector2D& OutSize,
                                  FVector2D& OutOffset,
                                  FVector2D& OutPaddingMin,
                                  FVector2D& OutPaddingMax) const
{
	// 当前以零尺寸锚点计算对齐偏移。
	FVector2D AllottedSize = FVector2D::ZeroVector;

	// 使用画布宿主的期望尺寸作为指示器尺寸。
	TSharedPtr<SWidget> CanvasHost = Indicator->CanvasHost.Pin();
	if (CanvasHost.IsValid())
	{
		OutSize = CanvasHost->GetDesiredSize();
	}

	// 根据水平对齐方式分配左右留白。
	switch (Indicator->IndicatorDescriptorParameter.HAlignment)
	{
	case HAlign_Left: // 与垂直顶部对齐使用相同的偏移规则。
		OutOffset.X = 0.0f;
		OutPaddingMin.X = 0.0f;
		OutPaddingMax.X = OutSize.X;
		break;

	case HAlign_Center:
		OutOffset.X = (AllottedSize.X - OutSize.X) / 2.0f;
		OutPaddingMin.X = OutSize.X / 2.0f;
		OutPaddingMax.X = OutPaddingMin.X;
		break;

	case HAlign_Right: // 与垂直底部对齐使用相同的偏移规则。
		OutOffset.X = AllottedSize.X - OutSize.X;
		OutPaddingMin.X = OutSize.X;
		OutPaddingMax.X = 0.0f;
		break;
	}

	// 根据垂直对齐方式分配上下留白。
	switch (Indicator->IndicatorDescriptorParameter.VAlignment)
	{
	case VAlign_Top:
		OutOffset.Y = 0.0f;
		OutPaddingMin.Y = 0.0f;
		OutPaddingMax.Y = OutSize.Y;
		break;

	case VAlign_Center:
		OutOffset.Y = (AllottedSize.Y - OutSize.Y) / 2.0f;
		OutPaddingMin.Y = OutSize.Y / 2.0f;
		OutPaddingMax.Y = OutPaddingMin.Y;
		break;

	case VAlign_Bottom:
		OutOffset.Y = AllottedSize.Y - OutSize.Y;
		OutPaddingMin.Y = OutSize.Y;
		OutPaddingMax.Y = 0.0f;
		break;
	}
}

void SActorIcon::UpdateActiveTimer()
{
	const bool NeedsTicks = AllIndicators.Num() > 0 || !IndicatorComponentPtr.IsValid();

	if (NeedsTicks && !TickHandle.IsValid())
	{
		TickHandle = RegisterActiveTimer(0, FWidgetActiveTimerDelegate::CreateSP(this, &SActorIcon::UpdateCanvas));
	}
}
