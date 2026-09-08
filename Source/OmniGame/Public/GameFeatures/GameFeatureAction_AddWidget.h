// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "Extension/AdvancedUIExtensionHelper.h"
#include "GameFeatures/GameFeatureAction_WorldActionBase.h"

#include "GameFeatureAction_AddWidget.generated.h"

#define UE_API OMNIGAME_API

struct FWorldContext;
struct FComponentRequestHandle;

USTRUCT()
struct FHUDLayoutRequest
{
	GENERATED_BODY()

	// 要生成的布局控件类
	UPROPERTY(EditAnywhere, Category=UI, meta=(AssetBundles="Client"))
	TSoftClassPtr<UCommonActivatableWidget> LayoutClass;

	// 布局要压入的 UI 层（Omni 的层根 Tag 为 AdvancedUI.UIStack）
	UPROPERTY(EditAnywhere, Category=UI, meta=(Categories="AdvancedUI.UIStack"))
	FGameplayTag LayerID;
};


USTRUCT()
struct FHUDElementEntry
{
	GENERATED_BODY()

	// 要生成的控件类
	UPROPERTY(EditAnywhere, Category=UI, meta=(AssetBundles="Client"))
	TSoftClassPtr<UUserWidget> WidgetClass;

	// 控件要放置的拓展点（Slot）Tag
	UPROPERTY(EditAnywhere, Category = UI)
	FGameplayTag SlotID;
};

/**
 * 向 HUD 添加布局与小部件的 GameFeatureAction
 *
 * 职责：
 * - Layout：功能激活且玩家 HUD 就绪时，把布局控件压入对应 UI 层（AdvancedUI.UIStack.*），
 *   反激活时对压入的控件调用 DeactivateWidget 回收
 * - Widgets：向 UAdvancedUIExtensionManager 按玩家上下文注册控件拓展，
 *   反激活时注销拓展句柄
 *
 * 注意：
 * - 只在客户端有意义；处理器的监听对象是 AOmniHUD
 *
 * 与 Lyra 的差异:
 * 1. 类名 UGameFeatureAction_AddWidgets（文件 AddWidget）统一为 UGameFeatureAction_AddWidget，
 *    结构体 FLyraHUDLayoutRequest / FLyraHUDElementEntry 去掉 Lyra 前缀
 * 2. 布局压层用 UUIHelperFunctions::PushWidgetToLayerForPlayer（AdvancedUI），
 *    对应 Lyra 的 UCommonUIExtensions::PushContentToLayer_ForPlayer
 * 3. 控件拓展用 UAdvancedUIExtensionManager（AdvancedUIExtension），
 *    对应 Lyra 的 UUIExtensionSubsystem；HUD 目标类用 AOmniHUD 对应 ALyraHUD
 * 4. LayerID 的 Tag 过滤根从 "UI.Layer" 换成本项目的 "AdvancedUI.UIStack"
 */
UCLASS(MinimalAPI, meta = (DisplayName = "添加小部件"))
class UGameFeatureAction_AddWidget final : public UGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

public:
	//~UGameFeatureAction interface
	UE_API virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
#if WITH_EDITORONLY_DATA
	UE_API virtual void AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData) override;
#endif
	//~End of UGameFeatureAction interface

	//~UObject interface
#if WITH_EDITOR
	UE_API virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~End of UObject interface

private:
	// 要压入 HUD 的布局列表
	UPROPERTY(EditAnywhere, Category=UI, meta=(TitleProperty="{LayerID} -> {LayoutClass}"))
	TArray<FHUDLayoutRequest> Layout;

	// 要加进 HUD 的小部件列表
	UPROPERTY(EditAnywhere, Category=UI, meta=(TitleProperty="{SlotID} -> {WidgetClass}"))
	TArray<FHUDElementEntry> Widgets;

	// 单个 HUD 上添加过的内容记录，反激活时据此回收
	struct FPerActorData
	{
		TArray<TWeakObjectPtr<UCommonActivatableWidget>> LayoutsAdded;
		TArray<FUIExtensionHandle> ExtensionHandles;
	};

	struct FPerContextData
	{
		TArray<TSharedPtr<FComponentRequestHandle>> ComponentRequests;
		TMap<FObjectKey, FPerActorData> ActorData;
	};

	TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;

	//~UGameFeatureAction_WorldActionBase interface
	UE_API virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;
	//~End of UGameFeatureAction_WorldActionBase interface

	UE_API void Reset(FPerContextData& ActiveData);
	UE_API void HandleActorExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext);
	UE_API void AddWidgets(AActor* Actor, FPerContextData& ActiveData);
	UE_API void RemoveWidgets(AActor* Actor, FPerContextData& ActiveData);
};

#undef UE_API
