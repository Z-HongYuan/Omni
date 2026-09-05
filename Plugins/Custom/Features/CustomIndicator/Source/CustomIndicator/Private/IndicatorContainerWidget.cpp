// Copyright © 2026 张鸿源. All Rights Reserved.


#include "IndicatorContainerWidget.h"
#include "SActorIcon.h"
#include "Widgets/Layout/SBox.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(IndicatorContainerWidget)

UIndicatorContainerWidget::UIndicatorContainerWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bIsVariable = true;

	UWidget::SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UIndicatorContainerWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyActorIcon.Reset();
}

TSharedRef<SWidget> UIndicatorContainerWidget::RebuildWidget()
{
	if (!IsDesignTime())
	{
		ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
		if (ensureMsgf(LocalPlayer, TEXT("Attempting to rebuild a UIndicatorContainerWidget without a valid LocalPlayer!")))
		{
			MyActorIcon = SNew(SActorIcon, FLocalPlayerContext(LocalPlayer), &ArrowBrush);
			return MyActorIcon.ToSharedRef();
		}
	}

	// 给它一个简单的框，NullWidget 在 UWidget 上使用并不安全
	return SNew(SBox);
}
