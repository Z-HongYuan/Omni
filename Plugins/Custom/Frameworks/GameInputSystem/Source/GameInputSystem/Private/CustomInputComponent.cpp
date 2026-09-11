// Copyright © 2026 张鸿源. All Rights Reserved.


#include "CustomInputComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CustomInputComponent)

UCustomInputComponent::UCustomInputComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCustomInputComponent::AddInputMappings(const UCustomInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);
	// 如果需要，你可以在这里处理任何自定义逻辑，从输入配置中添加某些内容
}

void UCustomInputComponent::RemoveInputMappings(const UCustomInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);
	// 在这里，您可以处理任何自定义逻辑，以移除您可能在上面添加的输入映射
}

void UCustomInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
	for (uint32 Handle : BindHandles)
	{
		RemoveBindingByHandle(Handle);
	}
	BindHandles.Reset();
}
