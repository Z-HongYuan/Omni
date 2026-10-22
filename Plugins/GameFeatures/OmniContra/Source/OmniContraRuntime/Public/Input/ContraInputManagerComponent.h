// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Input/OmniInputManagerComponent.h"
#include "Math/Vector2D.h"
#include "ContraInputManagerComponent.generated.h"

#define UE_API OMNICONTRARUNTIME_API

struct FInputActionValue;

/**
 * Contra 的输入管理器。保留主项目的初始化、能力输入转发和绑定清理流程。
 * 原生方向输入只驱动 X 轴移动，Y 轴输入暂时仅供读取。
 */
UCLASS(MinimalAPI, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UContraInputManagerComponent : public UOmniInputManagerComponent
{
	GENERATED_BODY()

public:
	UE_API UContraInputManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	UE_API FVector2D GetDirectionalInput() const { return DirectionalInput; }

protected:
	UE_API virtual void BindNativeInputActions(UExtInputComponent* Input, const UExtInputConfig* Config, TArray<uint32>& Handles) override;
	UE_API virtual void Input_Move(const FInputActionValue& Value) override;
	UE_API virtual void Input_MoveCompleted(const FInputActionValue& Value);

private:
	/** 缓存原始方向输入；动作完成、取消或重新绑定时清零。 */
	FVector2D DirectionalInput = FVector2D::ZeroVector;
};

#undef UE_API
