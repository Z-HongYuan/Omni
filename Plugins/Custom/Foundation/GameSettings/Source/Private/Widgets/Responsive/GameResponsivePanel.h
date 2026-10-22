// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/PanelWidget.h"
#include "GameResponsivePanel.generated.h"

class UGameResponsivePanelSlot;

/** 按屏幕物理宽度切换横向排列或纵向堆叠的自适应面板。 */
UCLASS()
class UGameResponsivePanel : public UPanelWidget
{
	GENERATED_UCLASS_BODY()
	/** 添加子控件，并返回其自适应布局槽。 */
	UFUNCTION(BlueprintCallable, Category="Widget")
	UGameResponsivePanelSlot* AddChildToGameResponsivePanel(UWidget* Content);

#if WITH_EDITOR
	// UWidget 接口
	virtual const FText GetPaletteCategory() override;
	// 结束 UWidget 接口
#endif

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
	bool bCanStackVertically = true;

protected:
	// UPanelWidget
	virtual UClass* GetSlotClass() const override;
	virtual void OnSlotAdded(UPanelSlot* Slot) override;
	virtual void OnSlotRemoved(UPanelSlot* Slot) override;
	// 结束 UPanelWidget

protected:
	TSharedPtr<class SGameResponsivePanel> MyGameResponsivePanel;

protected:
	// UWidget 接口
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// 结束 UWidget 接口
};
