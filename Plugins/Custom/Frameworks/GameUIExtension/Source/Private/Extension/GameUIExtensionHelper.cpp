// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Extension/GameUIExtensionHelper.h"
#include "Extension/GameUIExtensionManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameUIExtensionHelper)

bool FUIExtensionPoint::DoesExtensionPassContract(const FUIExtension* Extension) const
{
	if (UObject* DataPtr = Extension->Data)
	{
		// 匹配双方上下文都无效,或者匹配双方上下文相等
		const bool bMatchesContext = (ContextObject.IsExplicitlyNull() && Extension->ContextObject.IsExplicitlyNull()) || ContextObject == Extension->ContextObject;

		// 确保上下文相符。
		if (bMatchesContext)
		{
			// 数据可以是该数据类型的字面类，也可以是该类类型的实例。
			const UClass* DataClass = DataPtr->IsA(UClass::StaticClass()) ? Cast<UClass>(DataPtr) : DataPtr->GetClass();
			for (const UClass* AllowedDataClass : AllowedDataClasses)
			{
				// 如果是子类或者实现其接口都可以被匹配上
				if (DataClass->IsChildOf(AllowedDataClass) || DataClass->ImplementsInterface(AllowedDataClass))
				{
					return true;
				}
			}
		}
	}

	return false;
}

void FUIExtensionPointHandle::Unregister()
{
	if (UGameUIExtensionManager* ExtensionSourcePtr = ExtensionSource.Get())
		ExtensionSourcePtr->UnregisterExtensionPoint(*this);
}

void FUIExtensionHandle::Unregister()
{
	if (UGameUIExtensionManager* ExtensionSourcePtr = ExtensionSource.Get())
		ExtensionSourcePtr->UnregisterExtension(*this);
}

void UUIExtensionHandleFunctions::Unregister(FUIExtensionHandle& Handle)
{
	Handle.Unregister();
}

bool UUIExtensionHandleFunctions::IsValid(FUIExtensionHandle& Handle)
{
	return Handle.IsValid();
}

void UUIExtensionPointHandleFunctions::Unregister(FUIExtensionPointHandle& Handle)
{
	Handle.Unregister();
}

bool UUIExtensionPointHandleFunctions::IsValid(FUIExtensionPointHandle& Handle)
{
	return Handle.IsValid();
}
