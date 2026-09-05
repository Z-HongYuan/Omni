// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "UObject/Interface.h"
#include "IndicatorWidgetInterface.generated.h"

#define UE_API CUSTOMINDICATOR_API

class UIndicatorDescriptorDataObj;

/**
 * 当指示器控件实现此接口时, 可以获取到指示器数据上下文
 * 在绑定和解绑时会通过函数传递控件对应的数据上下文
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UIndicatorWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

class IIndicatorWidgetInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Indicator")
	void BindIndicator(UIndicatorDescriptorDataObj* Indicator);

	UFUNCTION(BlueprintNativeEvent, Category = "Indicator")
	void UnbindIndicator(const UIndicatorDescriptorDataObj* Indicator);
};

#undef UE_API
