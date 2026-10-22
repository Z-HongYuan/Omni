// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Templates/SubclassOf.h"
#include "UObject/Object.h"
#include "UObject/SoftObjectPtr.h"
#include "GameUIPolicy.generated.h"

#define UE_API GAMEUI_API

class ULocalPlayer;
class UGameUIRootWidget;
class UGameUIManager;

/** 本地多人游戏的 UI 控制模式。 */
UENUM()
enum class ELocalMultiplayerViewMode : uint8
{
	/** 仅主要玩家控制界面。 */
	PrimaryOnly,

	/** 各玩家共享全屏视口，通过休眠状态切换界面控制权。 */
	SingleToggle,

	/** 允许多个本地玩家同时控制各自界面。 */
	Simultaneous
};

/** 本地玩家及其根布局的视口状态。 */
USTRUCT()
struct FRootViewportLayoutInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> LocalPlayer = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UGameUIRootWidget> RootLayout = nullptr;

	UPROPERTY(Transient)
	bool bAddedToViewport = false;

	FRootViewportLayoutInfo() { ; }

	FRootViewportLayoutInfo(ULocalPlayer* InLocalPlayer, UGameUIRootWidget* InRootLayout, bool bIsInViewport) : LocalPlayer(InLocalPlayer), RootLayout(InRootLayout), bAddedToViewport(bIsInViewport) { ; }

	bool operator==(const ULocalPlayer* OtherLocalPlayer) const { return LocalPlayer == OtherLocalPlayer; }
};

/** 管理各本地玩家的根布局；由 GameUIManager 持有当前生效的策略。 */
UCLASS(MinimalAPI, Abstract, Blueprintable, Within = GameUIManager)
class UGameUIPolicy : public UObject
{
	GENERATED_BODY()

public:
	/** 从世界上下文获取当前生效的 UI 策略。 */
	static UE_API UGameUIPolicy* GetGameUIPolicy(const UObject* WorldContextObject);

	template <typename GameUIPolicyClass = UGameUIPolicy>
	static GameUIPolicyClass* GetGameUIPolicyAs(const UObject* WorldContextObject)
	{
		return Cast<GameUIPolicyClass>(GetGameUIPolicy(WorldContextObject));
	}

	UE_API virtual UWorld* GetWorld() const override;
	UE_API UGameUIManager* GetOwningUIManager() const;

	/** 获取指定本地玩家的根布局，尚未创建时返回空指针。 */
	UE_API UGameUIRootWidget* GetRootLayoutWidget(const ULocalPlayer* LocalPlayer) const;

	/** 获取本地多人游戏的 UI 控制模式。 */
	ELocalMultiplayerViewMode GetLocalMultiplayerViewMode() const { return LocalMultiplayerInteractionMode; }

	/** 仅在 SingleToggle 模式下，将控制权从当前活跃布局切换给指定的休眠布局。 */
	UE_API void RequestPrimaryControl(UGameUIRootWidget* Layout);

protected:
	UE_API void AddLayoutToViewport(ULocalPlayer* LocalPlayer, UGameUIRootWidget* Layout);
	UE_API void RemoveLayoutFromViewport(ULocalPlayer* LocalPlayer, UGameUIRootWidget* Layout);

	UE_API virtual void OnRootLayoutAddedToViewport(ULocalPlayer* LocalPlayer, UGameUIRootWidget* Layout);
	UE_API virtual void OnRootLayoutRemovedFromViewport(ULocalPlayer* LocalPlayer, UGameUIRootWidget* Layout);
	UE_API virtual void OnRootLayoutReleased(ULocalPlayer* LocalPlayer, UGameUIRootWidget* Layout);

	UE_API void CreateLayoutWidget(ULocalPlayer* LocalPlayer);
	UE_API TSubclassOf<UGameUIRootWidget> GetLayoutWidgetClass();

private:
	ELocalMultiplayerViewMode LocalMultiplayerInteractionMode = ELocalMultiplayerViewMode::PrimaryOnly;

	/** 各本地玩家创建根布局时使用的控件类。 */
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UGameUIRootWidget> LayoutClass;

	UPROPERTY(Transient)
	TArray<FRootViewportLayoutInfo> RootViewportLayouts;

	/** 由 GameUIManager 通知玩家加入，绑定 Controller 变化并创建或恢复布局。 */
	UE_API void NotifyPlayerAdded(ULocalPlayer* LocalPlayer);
	/** 从视口移除布局，但保留记录以便重新加入。 */
	UE_API void NotifyPlayerRemoved(ULocalPlayer* LocalPlayer);
	/** 解除玩家事件绑定，并释放其布局记录。 */
	UE_API void NotifyPlayerDestroyed(ULocalPlayer* LocalPlayer);

	friend class UGameUIManager;
};

#undef UE_API
