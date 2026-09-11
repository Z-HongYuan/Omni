// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "AdvancedUIExtensionHelper.h"
#include "AdvancedUIExtensionManager.generated.h"

#define UE_API GAMEUIEXTENSION_API

class UUserWidget;

/**
 * 
 */
UCLASS(MinimalAPI)
class UAdvancedUIExtensionManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	// UE_API virtual void Deinitialize() override;

	// 使用必要数据注册一个拓展点
	UE_API FUIExtensionPointHandle RegisterExtensionPoint(const FGameplayTag& ExtensionPointTag,
	                                                      EUIExtensionPointMatch ExtensionPointTagMatchType,
	                                                      const TArray<UClass*>& AllowedDataClasses,
	                                                      const FExtendExtensionPointDelegate& ExtensionCallback);

	// 使用必要数据+上下文注册一个拓展点
	UE_API FUIExtensionPointHandle RegisterExtensionPointForContext(const FGameplayTag& ExtensionPointTag,
	                                                                UObject* ContextObject,
	                                                                EUIExtensionPointMatch ExtensionPointTagMatchType,
	                                                                const TArray<UClass*>& AllowedDataClasses,
	                                                                FExtendExtensionPointDelegate ExtensionCallback);

	// 将拓展注入到拓展点中,使用控件注入
	UE_API FUIExtensionHandle RegisterExtensionAsWidget(const FGameplayTag& ExtensionPointTag, TSubclassOf<UUserWidget> WidgetClass, int32 Priority);
	// 将拓展注入到拓展点中,使用控件注入(需要上下文匹配)
	UE_API FUIExtensionHandle RegisterExtensionAsWidgetForContext(const FGameplayTag& ExtensionPointTag, UObject* ContextObject, TSubclassOf<UUserWidget> WidgetClass, int32 Priority);
	// 将拓展注入到拓展点中,使用数据对象注入
	UE_API FUIExtensionHandle RegisterExtensionAsData(const FGameplayTag& ExtensionPointTag, UObject* ContextObject, UObject* Data, int32 Priority);

	// 注销拓展
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension")
	UE_API void UnregisterExtension(const FUIExtensionHandle& ExtensionHandle);

	// 注销拓展点
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension")
	UE_API void UnregisterExtensionPoint(const FUIExtensionPointHandle& ExtensionPointHandle);

	// 添加引用防止GC
	static UE_API void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);

protected:
	// 向所有的拓展检测一遍 新拓展点注册
	UE_API void NotifyExtensionsOfExtensions(TSharedPtr<FUIExtensionPoint>& ExtensionPoint);
	// 向所有的拓展点检测一遍 新拓展注册
	UE_API void NotifyExtensionPointsOfExtension(EUIExtensionAction Action, TSharedPtr<FUIExtension>& Extension);

	// 蓝图使用的函数,用于向 Manager 注册一个拓展点
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="UI Extension", meta = (DisplayName = "Register Extension Point"))
	UE_API FUIExtensionPointHandle K2_RegisterExtensionPoint(FGameplayTag ExtensionPointTag,
	                                                         EUIExtensionPointMatch ExtensionPointTagMatchType,
	                                                         const TArray<UClass*>& AllowedDataClasses,
	                                                         FExtendExtensionPointDynamicDelegate ExtensionCallback);

	// 蓝图使用的函数,用于向 Manager 注册一个拓展 使用 Widget 类型
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension", meta = (DisplayName = "Register Extension (Widget)"))
	UE_API FUIExtensionHandle K2_RegisterExtensionAsWidget(FGameplayTag ExtensionPointTag, TSubclassOf<UUserWidget> WidgetClass, int32 Priority = -1);

	// 蓝图使用的函数,用于向 Manager 注册一个拓展 使用 Widget 类型 并且检查对方(拓展点)上下文是否符合
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension", meta = (DisplayName = "Register Extension (Widget For Context)"))
	UE_API FUIExtensionHandle K2_RegisterExtensionAsWidgetForContext(FGameplayTag ExtensionPointTag, TSubclassOf<UUserWidget> WidgetClass, UObject* ContextObject, int32 Priority = -1);

	// 蓝图使用的函数,用于向 Manager 注册一个拓展 使用 数据(Data) 类型
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="UI Extension", meta = (DisplayName = "Register Extension (Data)"))
	UE_API FUIExtensionHandle K2_RegisterExtensionAsData(FGameplayTag ExtensionPointTag, UObject* Data, int32 Priority = -1);

	// 蓝图使用的函数,用于向 Manager 注册一个拓展 使用 数据(Data) 类型 并且检查对方(拓展点)上下文是否符合
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="UI Extension", meta = (DisplayName = "Register Extension (Data For Context)"))
	UE_API FUIExtensionHandle K2_RegisterExtensionAsDataForContext(FGameplayTag ExtensionPointTag, UObject* ContextObject, UObject* Data, int32 Priority = -1);

	UE_API FUIExtensionRequest CreateExtensionRequest(const TSharedPtr<FUIExtension>& Extension);

private:
	typedef TArray<TSharedPtr<FUIExtensionPoint>> FExtensionPointList;
	TMap<FGameplayTag, FExtensionPointList> ExtensionPointMap;

	typedef TArray<TSharedPtr<FUIExtension>> FExtensionList;
	TMap<FGameplayTag, FExtensionList> ExtensionMap;
};
#undef UE_API
