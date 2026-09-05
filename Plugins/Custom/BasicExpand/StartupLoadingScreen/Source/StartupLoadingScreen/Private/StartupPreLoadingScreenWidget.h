// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Widgets/SCompoundWidget.h"

/*
 * 提供最基础的预加载界面,黑屏
 */
class SStartupPreLoadingScreenWidget : public SCompoundWidget, public FGCObject
{
public:
	SLATE_BEGIN_ARGS(SStartupPreLoadingScreenWidget)
		{
		}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	//~ Begin FGCObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;
	//~ End FGCObject interface
};
