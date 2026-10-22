// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/Widget.h"
#include "IndicatorContainerWidget.generated.h"

#define UE_API CUSTOMINDICATOR_API

class SActorIcon;

/**
 * 指示器 Slate 面板的 UMG 包装器。
 * 运行时从所属本地玩家的 PlayerController 获取管理组件，监听指示器增删。
 */
UCLASS(MinimalAPI)
class UIndicatorContainerWidget : public UWidget
{
	GENERATED_BODY()

public:
	UIndicatorContainerWidget(const FObjectInitializer& ObjectInitializer);

	/** 指示器被限制在屏幕边缘时使用的箭头画刷。 */
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
