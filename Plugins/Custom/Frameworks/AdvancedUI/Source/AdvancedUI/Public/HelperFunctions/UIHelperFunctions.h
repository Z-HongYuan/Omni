// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "UIHelperFunctions.generated.h"

#define UE_API ADVANCEDUI_API

class ULocalPlayer;
class UUserWidget;
struct FGameplayTag;
class UCommonActivatableWidget;
enum class ECommonInputType : uint8;

/**
 * 
 */
UCLASS(MinimalAPI)
class UUIHelperFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UUIHelperFunctions() { ; }

	// 在控件蓝图中获取当前的输入模式 (键鼠/手柄/...)
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "AdvancedUI", meta = (WorldContext = "WidgetContextObject"))
	static UE_API ECommonInputType GetOwningPlayerInputType(const UUserWidget* WidgetContextObject);

	// 在控件蓝图中获取当前的输入模式是否为触摸
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "AdvancedUI", meta = (WorldContext = "WidgetContextObject"))
	static UE_API bool IsOwningPlayerUsingTouch(const UUserWidget* WidgetContextObject);

	// 在控件蓝图中获取当前的输入模式是否为手柄
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "AdvancedUI", meta = (WorldContext = "WidgetContextObject"))
	static UE_API bool IsOwningPlayerUsingGamepad(const UUserWidget* WidgetContextObject);

	// 同步推送控件到指定本地玩家 并获取控件引用
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "AdvancedUI")
	static UE_API UCommonActivatableWidget* PushWidgetToLayerForPlayer(const ULocalPlayer* LocalPlayer,
	                                                                   UPARAM(meta = (Categories = "AdvancedUI.UIStack")) FGameplayTag LayerName,
	                                                                   UPARAM(meta = (AllowAbstract = false)) TSubclassOf<UCommonActivatableWidget> WidgetClass);

	// 异步推送控件到指定本地玩家
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "AdvancedUI")
	static UE_API void PushSoftWidgetToLayerForPlayer(const ULocalPlayer* LocalPlayer,
	                                                  UPARAM(meta = (Categories = "AdvancedUI.UIStack")) FGameplayTag LayerName,
	                                                  UPARAM(meta = (AllowAbstract = false)) TSoftClassPtr<UCommonActivatableWidget> WidgetClass);

	// 在所有堆栈中寻找并删除指定的控件
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "AdvancedUI")
	static UE_API void RemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget);

	// 从控制器中获取本地玩家
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "AdvancedUI")
	static UE_API ULocalPlayer* GetLocalPlayerFromController(APlayerController* PlayerController);

	// 有原因的暂停玩家输入 本质上是使用 UCommonInputSubsystem 过滤了所有输入
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "AdvancedUI")
	static UE_API FName SuspendInputForPlayer(APlayerController* PlayerController, FName SuspendReason);
	// 有原因的暂停玩家输入 本质上是使用 UCommonInputSubsystem 过滤了所有输入
	static UE_API FName SuspendInputForPlayer(const ULocalPlayer* LocalPlayer, FName SuspendReason);

	// 有原因的恢复玩家输入 本质上是使用 UCommonInputSubsystem 过滤了所有输入
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "AdvancedUI")
	static UE_API void ResumeInputForPlayer(APlayerController* PlayerController, FName SuspendToken);
	// 有原因的恢复玩家输入 本质上是使用 UCommonInputSubsystem 过滤了所有输入
	static UE_API void ResumeInputForPlayer(const ULocalPlayer* LocalPlayer, FName SuspendToken);

private:
	static UE_API int32 InputSuspensions;
};

#undef UE_API
