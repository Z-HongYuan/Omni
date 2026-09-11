// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Input/ExtInputConfig.h"
#include "EnhancedInputComponent.h"
#include "ExtInputComponent.generated.h"

#define UE_API ABILITYEXTENSION_API

struct FGameplayTag;
class UExtInputConfig;
class UEnhancedInputLocalPlayerSubsystem;

/*
 * 自定义的增强输入插件,支持输入与Tag之间绑定
 * 用于使用输入配置数据资产管理输入映射和绑定。
 */
UCLASS(MinimalAPI, ClassGroup=(Input))
class UExtInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	UE_API UExtInputComponent();

	UE_API void AddInputMappings(const UExtInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;
	UE_API void RemoveInputMappings(const UExtInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;

	template <class UserClass, typename FuncType>
	void BindNativeAction(const UExtInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound)
	{
		check(InputConfig);
		if (const UInputAction* IA = InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound))
		{
			BindAction(IA, TriggerEvent, Object, Func);
		}
	};

	template <class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	void BindAbilityActions(const UExtInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles)
	{
		check(InputConfig);

		for (const FExtInputAction& Action : InputConfig->AbilityInputActions)
		{
			if (Action.InputAction && Action.InputTag.IsValid())
			{
				if (PressedFunc)
				{
					BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, PressedFunc, Action.InputTag).GetHandle());
				}

				if (ReleasedFunc)
				{
					BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag).GetHandle());
				}
			}
		}
	}

	UE_API void RemoveBinds(TArray<uint32>& BindHandles);
};

#undef UE_API
