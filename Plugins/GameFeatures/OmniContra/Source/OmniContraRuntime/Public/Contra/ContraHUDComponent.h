// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/ControllerComponent.h"
#include "ContraHUDComponent.generated.h"

class UContraHUDWidget;

/** GF 注入到 PC 的本地 HUD 接线；退出时移除自己创建的控件。 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OMNICONTRARUNTIME_API UContraHUDComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type Reason) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UContraHUDWidget> Widget;
	bool bPreviousMouseCursor = false;
};
