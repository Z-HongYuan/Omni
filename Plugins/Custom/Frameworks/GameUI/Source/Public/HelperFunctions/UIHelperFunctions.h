// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "UIHelperFunctions.generated.h"

#define UE_API GAMEUI_API

class ULocalPlayer;
class UUserWidget;
struct FGameplayTag;
class UCommonActivatableWidget;
enum class ECommonInputType : uint8;

/** 提供玩家输入类型查询、UI 分层操作和输入暂停接口。 */
UCLASS(MinimalAPI)
class UUIHelperFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UUIHelperFunctions() { ; }

	/** 查询控件所属本地玩家的输入设备类型；无法查询时返回 Count。 */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "GameUI", meta = (WorldContext = "WidgetContextObject"))
	static UE_API ECommonInputType GetOwningPlayerInputType(const UUserWidget* WidgetContextObject);

	/** 查询控件所属玩家是否使用触摸输入。 */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "GameUI", meta = (WorldContext = "WidgetContextObject"))
	static UE_API bool IsOwningPlayerUsingTouch(const UUserWidget* WidgetContextObject);

	/** 查询控件所属玩家是否使用手柄输入。 */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "GameUI", meta = (WorldContext = "WidgetContextObject"))
	static UE_API bool IsOwningPlayerUsingGamepad(const UUserWidget* WidgetContextObject);

	/** 同步推送控件到指定本地玩家的 UI 层，失败时返回空指针。 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GameUI")
	static UE_API UCommonActivatableWidget* PushWidgetToLayerForPlayer(const ULocalPlayer* LocalPlayer,
	                                                                   UPARAM(meta = (Categories = "GameUI.UIStack")) FGameplayTag LayerName,
	                                                                   UPARAM(meta = (AllowAbstract = false)) TSubclassOf<UCommonActivatableWidget> WidgetClass);

	/** 异步推送控件到指定本地玩家的 UI 层，并在加载期间暂停输入。 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GameUI")
	static UE_API void PushSoftWidgetToLayerForPlayer(const ULocalPlayer* LocalPlayer,
	                                                  UPARAM(meta = (Categories = "GameUI.UIStack")) FGameplayTag LayerName,
	                                                  UPARAM(meta = (AllowAbstract = false)) TSoftClassPtr<UCommonActivatableWidget> WidgetClass);

	/** 从所属玩家的各个 UI 堆栈中移除指定控件。 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GameUI")
	static UE_API void RemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget);

	/** 从控制器获取本地玩家；控制器无效时返回空指针。 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GameUI")
	static UE_API ULocalPlayer* GetLocalPlayerFromController(APlayerController* PlayerController);

	/** 为玩家的各类输入添加过滤令牌；调用方须保存令牌并在结束时恢复。 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GameUI")
	static UE_API FName SuspendInputForPlayer(APlayerController* PlayerController, FName SuspendReason);
	/** 为玩家的各类输入添加过滤令牌；调用方须保存令牌并在结束时恢复。 */
	static UE_API FName SuspendInputForPlayer(const ULocalPlayer* LocalPlayer, FName SuspendReason);

	/** 移除指定令牌的输入过滤；其他暂停令牌仍然有效。 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "GameUI")
	static UE_API void ResumeInputForPlayer(APlayerController* PlayerController, FName SuspendToken);
	/** 移除指定令牌的输入过滤；其他暂停令牌仍然有效。 */
	static UE_API void ResumeInputForPlayer(const ULocalPlayer* LocalPlayer, FName SuspendToken);

private:
	static UE_API int32 InputSuspensions;
};

#undef UE_API
