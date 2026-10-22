// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "GameUIExtensionHelper.h"
#include "GameUIExtensionManager.generated.h"

#define UE_API GAMEUIEXTENSION_API

class UUserWidget;

/** 按标签、上下文和数据类型匹配 UI 扩展与扩展点，并分发添加、移除通知。 */
UCLASS(MinimalAPI)
class UGameUIExtensionManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	// UE_API virtual void Deinitialize() override;

	/** 注册无上下文的扩展点，并立即通知已匹配的扩展。 */
	UE_API FUIExtensionPointHandle RegisterExtensionPoint(const FGameplayTag& ExtensionPointTag,
	                                                      EUIExtensionPointMatch ExtensionPointTagMatchType,
	                                                      const TArray<UClass*>& AllowedDataClasses,
	                                                      const FExtendExtensionPointDelegate& ExtensionCallback);

	/** 注册指定上下文的扩展点；回调可能在本次注册返回前执行。 */
	UE_API FUIExtensionPointHandle RegisterExtensionPointForContext(const FGameplayTag& ExtensionPointTag,
	                                                                UObject* ContextObject,
	                                                                EUIExtensionPointMatch ExtensionPointTagMatchType,
	                                                                const TArray<UClass*>& AllowedDataClasses,
	                                                                FExtendExtensionPointDelegate ExtensionCallback);

	/** 注册无上下文的控件类扩展。 */
	UE_API FUIExtensionHandle RegisterExtensionAsWidget(const FGameplayTag& ExtensionPointTag, TSubclassOf<UUserWidget> WidgetClass, int32 Priority);
	/** 注册指定上下文的控件类扩展。 */
	UE_API FUIExtensionHandle RegisterExtensionAsWidgetForContext(const FGameplayTag& ExtensionPointTag, UObject* ContextObject, TSubclassOf<UUserWidget> WidgetClass, int32 Priority);
	/** 注册数据扩展，并通知符合标签、上下文和类型条件的扩展点。 */
	UE_API FUIExtensionHandle RegisterExtensionAsData(const FGameplayTag& ExtensionPointTag, UObject* ContextObject, UObject* Data, int32 Priority);

	/** 注销本管理器创建的扩展，并先通知接收方移除。 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension")
	UE_API void UnregisterExtension(const FUIExtensionHandle& ExtensionHandle);

	/** 注销本管理器创建的扩展点。 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension")
	UE_API void UnregisterExtensionPoint(const FUIExtensionPointHandle& ExtensionPointHandle);

	/** 为共享记录中的数据对象和允许类型收集 GC 引用。 */
	static UE_API void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);

protected:
	/** 新扩展点注册后，回放当前匹配扩展的添加通知。 */
	UE_API void NotifyExtensionsOfExtensions(TSharedPtr<FUIExtensionPoint>& ExtensionPoint);
	/** 向匹配的扩展点分发此扩展的添加或移除通知。 */
	UE_API void NotifyExtensionPointsOfExtension(EUIExtensionAction Action, TSharedPtr<FUIExtension>& Extension);

	/** 为蓝图注册扩展点，以弱对象回调转发通知。 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="UI Extension", meta = (DisplayName = "Register Extension Point"))
	UE_API FUIExtensionPointHandle K2_RegisterExtensionPoint(FGameplayTag ExtensionPointTag,
	                                                         EUIExtensionPointMatch ExtensionPointTagMatchType,
	                                                         const TArray<UClass*>& AllowedDataClasses,
	                                                         FExtendExtensionPointDynamicDelegate ExtensionCallback);

	/** 为蓝图注册无上下文的控件类扩展。 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension", meta = (DisplayName = "Register Extension (Widget)"))
	UE_API FUIExtensionHandle K2_RegisterExtensionAsWidget(FGameplayTag ExtensionPointTag, TSubclassOf<UUserWidget> WidgetClass, int32 Priority = -1);

	/** 为蓝图注册指定上下文的控件类扩展；上下文不可为空。 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension", meta = (DisplayName = "Register Extension (Widget For Context)"))
	UE_API FUIExtensionHandle K2_RegisterExtensionAsWidgetForContext(FGameplayTag ExtensionPointTag, TSubclassOf<UUserWidget> WidgetClass, UObject* ContextObject, int32 Priority = -1);

	/** 为蓝图注册无上下文的数据扩展。 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="UI Extension", meta = (DisplayName = "Register Extension (Data)"))
	UE_API FUIExtensionHandle K2_RegisterExtensionAsData(FGameplayTag ExtensionPointTag, UObject* Data, int32 Priority = -1);

	/** 为蓝图注册指定上下文的数据扩展；上下文不可为空。 */
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
