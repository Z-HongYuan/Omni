// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Extension/GameUIExtensionHelper.h"
#include "Extension/GameUIExtensionManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameUIExtensionHelper)

bool FUIExtensionPoint::DoesExtensionPassContract(const FUIExtension* Extension) const
{
	if (UObject* DataPtr = Extension->Data)
	{
		// 仅双方都未指定上下文，或上下文弱引用相等时匹配。
		const bool bMatchesContext = (ContextObject.IsExplicitlyNull() && Extension->ContextObject.IsExplicitlyNull()) || ContextObject == Extension->ContextObject;

		// 上下文匹配后再验证数据类型。
		if (bMatchesContext)
		{
			// 数据既可以是 UClass，也可以是对象实例，统一取出待检验的类型。
			const UClass* DataClass = DataPtr->IsA(UClass::StaticClass()) ? Cast<UClass>(DataPtr) : DataPtr->GetClass();
			for (const UClass* AllowedDataClass : AllowedDataClasses)
			{
				// 接受允许类型的派生类，以及实现允许接口的类型。
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
