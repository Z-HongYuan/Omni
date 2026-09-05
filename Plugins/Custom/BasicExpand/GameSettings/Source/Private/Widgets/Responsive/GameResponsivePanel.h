// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/PanelWidget.h"
#include "GameResponsivePanel.generated.h"

class UGameResponsivePanelSlot;

/**
 * 允许控件按水平流程进行布局。
 *
 * * 多个子控件
 * * 水平流程
 */
UCLASS()
class UGameResponsivePanel : public UPanelWidget
{
	GENERATED_UCLASS_BODY()
	/**  */
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
