// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "UObject/GCObject.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 显示纯黑背景的启动界面。
 * 保留 FGCObject 接口，便于持有 UObject 资源时登记垃圾回收引用。
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
