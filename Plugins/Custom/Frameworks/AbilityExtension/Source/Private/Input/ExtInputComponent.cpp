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
	// 当前保留扩展入口，尚未添加输入映射逻辑。
}

void UExtInputComponent::RemoveInputMappings(const UExtInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);
	// 当前保留扩展入口，尚未添加输入映射移除逻辑。
}

void UExtInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
	for (uint32 Handle : BindHandles)
	{
		RemoveBindingByHandle(Handle);
	}
	BindHandles.Reset();
}
