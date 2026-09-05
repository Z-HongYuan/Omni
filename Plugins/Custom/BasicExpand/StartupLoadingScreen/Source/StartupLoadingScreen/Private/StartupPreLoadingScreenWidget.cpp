// Copyright © 2026 张鸿源. All Rights Reserved.

#include "StartupPreLoadingScreenWidget.h"
#include "Widgets/Layout/SBorder.h"

void SStartupPreLoadingScreenWidget::Construct(const FArguments& InArgs)
{
	// 构建一个黑屏边框,可以通过这个做其他的事情,比如构建一个小型Logo
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor::Black)
		.Padding(0)
	];
}

void SStartupPreLoadingScreenWidget::AddReferencedObjects(FReferenceCollector& Collector)
{
	//WidgetAssets.AddReferencedObjects(Collector);
}

FString SStartupPreLoadingScreenWidget::GetReferencerName() const
{
	return TEXT("SStartupPreLoadingScreenWidget");
}
