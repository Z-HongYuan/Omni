// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "IndicatorDescriptorDataObj.h"
#include "IndicatorManagerComponent.generated.h"

#define UE_API CUSTOMINDICATOR_API

/*
 * 用于注册和保存指示器的组件
 * 最好是附加到Controller上, 如果附加到其他Actor上后,需要手动变更指示器渲染系统的Component指针获取到正确的指示器组件实例
 * 此组件只是为了管理指示器的数据,不会负责指示器的显示
 */
UCLASS(MinimalAPI, ClassGroup=(Indicator), meta=(BlueprintSpawnableComponent))
class UIndicatorManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UE_API UIndicatorManagerComponent(const FObjectInitializer& ObjectInitializer);

	static UE_API UIndicatorManagerComponent* GetComponent(AController* Controller);
	UFUNCTION(BlueprintCallable, Category = Indicator)
	static UE_API UIndicatorManagerComponent* GetIndicatorManagerComponent(AController* Controller);

	UFUNCTION(BlueprintCallable, Category = Indicator)
	UE_API void AddIndicator(UIndicatorDescriptorDataObj* IndicatorDescriptor);

	UFUNCTION(BlueprintCallable, Category = Indicator)
	UE_API void RemoveIndicator(UIndicatorDescriptorDataObj* IndicatorDescriptor);

	DECLARE_EVENT_OneParam(UIndicatorManagerComponent, FIndicatorEvent, UIndicatorDescriptorDataObj* Descriptor)

	FIndicatorEvent OnIndicatorAdded;
	FIndicatorEvent OnIndicatorRemoved;

	// 获取所有指示器
	const TArray<UIndicatorDescriptorDataObj*>& GetIndicators() const { return Indicators; }

private:
	UPROPERTY()
	TArray<TObjectPtr<UIndicatorDescriptorDataObj>> Indicators;
};

#undef UE_API
