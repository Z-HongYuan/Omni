// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "UObject/GCObject.h"
#include "Widgets/SCompoundWidget.h"

/*
 * 提供可扩展的启动界面，目前显示纯黑背景。
 * 保留 FGCObject，用于后续持有纹理等 UObject 资源。
 */
class SStartupPreLoadingScreenWidget : public SCompoundWidget, public FGCObject
{
public:
	SLATE_BEGIN_ARGS(SStartupPreLoadingScreenWidget)
		{
		}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	//~FGCObject 接口
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;
	//~FGCObject 接口结束
};
