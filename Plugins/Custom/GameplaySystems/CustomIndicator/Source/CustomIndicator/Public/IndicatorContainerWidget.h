// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/Widget.h"
#include "IndicatorContainerWidget.generated.h"

#define UE_API CUSTOMINDICATOR_API

class SActorIcon;

/**
 * 指示器容器控件
 * 作为指示器面板的UMG包装器
 * 当控件被纳入UMG布局时,会自动监听指示器组件的事件,并且更新指示器
 * 内部会自动获取在PC上指示器组件,所以推荐将指示器组件添加进PC中
 */
UCLASS(MinimalAPI)
class UIndicatorContainerWidget : public UWidget
{
	GENERATED_BODY()

public:
	UIndicatorContainerWidget(const FObjectInitializer& ObjectInitializer);

	// 当UI被压缩到屏幕需要显示箭头时使用的默认箭头刷子
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance)
	FSlateBrush ArrowBrush;

protected:
	// UWidget interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End UWidget

protected:
	TSharedPtr<SActorIcon> MyActorIcon;
};

#undef UE_API
