// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "ContraHUDWidget.generated.h"

class UTextBlock;
class UButton;

/** 可替换的灰盒 HUD：读取当前 Pawn、PS 与 GS；不保存或计算玩法状态。 */
UCLASS()
class OMNICONTRARUNTIME_API UContraHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;

private:
	UFUNCTION()
	void RestartPressed();
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> Status;
	UPROPERTY(Transient)
	TObjectPtr<UButton> Restart;
};
