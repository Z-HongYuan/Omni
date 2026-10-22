// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "IndicatorDescriptorDataObj.h"
#include "IndicatorManagerComponent.generated.h"

#define UE_API CUSTOMINDICATOR_API

/**
 * 保存指示器描述对象并广播增删事件，具体显示由指示器面板负责。
 * 面板默认从本地 PlayerController 查找此组件；挂载到其他 Actor 时需另行接入。
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

	/** 返回当前注册的指示器描述对象。 */
	const TArray<UIndicatorDescriptorDataObj*>& GetIndicators() const { return Indicators; }

private:
	UPROPERTY()
	TArray<TObjectPtr<UIndicatorDescriptorDataObj>> Indicators;
};

#undef UE_API
