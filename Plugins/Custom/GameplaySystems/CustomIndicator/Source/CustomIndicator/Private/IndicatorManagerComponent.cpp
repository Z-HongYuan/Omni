// Copyright © 2026 张鸿源. All Rights Reserved.


#include "IndicatorManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(IndicatorManagerComponent)

UIndicatorManagerComponent::UIndicatorManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoRegister = true;
	bAutoActivate = true;
}

UIndicatorManagerComponent* UIndicatorManagerComponent::GetComponent(AController* Controller)
{
	if (Controller)
	{
		return Controller->FindComponentByClass<UIndicatorManagerComponent>();
	}

	return nullptr;
}

UIndicatorManagerComponent* UIndicatorManagerComponent::GetIndicatorManagerComponent(AController* Controller)
{
	return UIndicatorManagerComponent::GetComponent(Controller);
}

void UIndicatorManagerComponent::AddIndicator(UIndicatorDescriptorDataObj* IndicatorDescriptor)
{
	IndicatorDescriptor->SetIndicatorManagerComponent(this);
	OnIndicatorAdded.Broadcast(IndicatorDescriptor);
	Indicators.Add(IndicatorDescriptor);
}

void UIndicatorManagerComponent::RemoveIndicator(UIndicatorDescriptorDataObj* IndicatorDescriptor)
{
	if (IndicatorDescriptor)
	{
		ensure(IndicatorDescriptor->IndicatorDescriptorParameter.ManagerPtr == this);

		OnIndicatorRemoved.Broadcast(IndicatorDescriptor);
		Indicators.Remove(IndicatorDescriptor);
	}
}
