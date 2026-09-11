// Copyright © 2026 张鸿源. All Rights Reserved.

#include "StartupPreLoadingScreenWidget.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"

void SStartupPreLoadingScreenWidget::Construct(const FArguments& InArgs)
{
	// 提供纯黑背景，后续可在此扩展 Logo、图片或其他 Slate 控件。
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
	// 当前没有持有 UObject；后续添加纹理等资源时，在此通过 Collector 登记引用，防止被垃圾回收。
}

FString SStartupPreLoadingScreenWidget::GetReferencerName() const
{
	return TEXT("SStartupPreLoadingScreenWidget");
}
