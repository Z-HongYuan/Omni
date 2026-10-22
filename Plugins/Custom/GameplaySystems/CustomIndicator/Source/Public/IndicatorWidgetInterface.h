// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "UObject/Interface.h"
#include "IndicatorWidgetInterface.generated.h"

#define UE_API CUSTOMINDICATOR_API

class UIndicatorDescriptorDataObj;

/** 指示器控件通过绑定与解绑回调接收或释放描述对象关联。 */
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
