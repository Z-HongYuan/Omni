// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Input/ExtInputComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExtInputComponent)

UExtInputComponent::UExtInputComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UExtInputComponent::AddInputMappings(const UExtInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);
	// 如果需要，你可以在这里处理任何自定义逻辑，从输入配置中添加某些内容
}

void UExtInputComponent::RemoveInputMappings(const UExtInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);
	// 在这里，您可以处理任何自定义逻辑，以移除您可能在上面添加的输入映射
}

void UExtInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
	for (uint32 Handle : BindHandles)
	{
		RemoveBindingByHandle(Handle);
	}
	BindHandles.Reset();
}
