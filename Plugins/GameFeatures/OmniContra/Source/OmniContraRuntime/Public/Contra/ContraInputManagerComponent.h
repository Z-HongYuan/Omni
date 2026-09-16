// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Input/OmniInputManagerComponent.h"
#include "ContraInputManagerComponent.generated.h"

/** 横版原生输入：世界 X 轴移动，鼠标投影到战斗平面或右摇杆八方向瞄准。由 GF 添加。 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OMNICONTRARUNTIME_API UContraInputManagerComponent : public UOmniInputManagerComponent
{
	GENERATED_BODY()

public:
	UContraInputManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BindNativeInputActions(UExtInputComponent* Input, const UExtInputConfig* Config, TArray<uint32>& Handles) override;

private:
	void Move(const FInputActionValue& Value);
	void AimStick(const FInputActionValue& Value);
	void AimMouse(const FInputActionValue& Value);
	void SetAim(FVector2D Direction);
	bool bUseStickAim = false;
};
