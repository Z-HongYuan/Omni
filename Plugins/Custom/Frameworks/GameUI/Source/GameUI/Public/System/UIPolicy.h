// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Templates/SubclassOf.h"
#include "UObject/Object.h"
#include "UObject/SoftObjectPtr.h"
#include "UIPolicy.generated.h"

#define UE_API GAMEUI_API

class ULocalPlayer;
class UGameRootLayoutWidget;
class UUIManager;

/**
 * 屏幕模式
 */
UENUM()
enum class ELocalMultiplayerViewMode : uint8
{
	// 单人单屏
	PrimaryOnly,

	// 单人全屏视口，但玩家可以切换显示和休眠的控制权
	SingleToggle,

	// 多人同屏
	Simultaneous
};

/*每个本地玩家所拥有的信息*/
USTRUCT()
struct FRootViewportLayoutInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> LocalPlayer = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UGameRootLayoutWidget> RootLayout = nullptr;

	UPROPERTY(Transient)
	bool bAddedToViewport = false;

	FRootViewportLayoutInfo() { ; }

	FRootViewportLayoutInfo(ULocalPlayer* InLocalPlayer, UGameRootLayoutWidget* InRootLayout, bool bIsInViewport) : LocalPlayer(InLocalPlayer), RootLayout(InRootLayout), bAddedToViewport(bIsInViewport) { ; }

	bool operator==(const ULocalPlayer* OtherLocalPlayer) const { return LocalPlayer == OtherLocalPlayer; }
};

/**
 * 一个 UI 策略 包含全部的 LocalPlayer
 * 在游戏中有且仅有一个 UI 策略生效
 */
UCLASS(MinimalAPI, Abstract, Blueprintable, Within = UIManager)
class UUIPolicy : public UObject
{
	GENERATED_BODY()

public:
	// 从游戏上下文中获取当前的 UI 策略
	static UE_API UUIPolicy* GetGameUIPolicy(const UObject* WorldContextObject);

	template <typename GameUIPolicyClass = UUIPolicy>
	static GameUIPolicyClass* GetGameUIPolicyAs(const UObject* WorldContextObject)
	{
		return Cast<GameUIPolicyClass>(GetGameUIPolicy(WorldContextObject));
	}

	UE_API virtual UWorld* GetWorld() const override;
	UE_API UUIManager* GetOwningUIManager() const;

	// 获取指定 LocalPlayer 拥有的 RootWidget
	UE_API UGameRootLayoutWidget* GetRootLayoutWidget(const ULocalPlayer* LocalPlayer) const;

	// 获取 本地多玩家 的视口模式
	ELocalMultiplayerViewMode GetLocalMultiplayerViewMode() const { return LocalMultiplayerInteractionMode; }

	// 将指定的 RootWidget 设置为主要的视口屏幕 (将会隐藏/休眠其他所有的 RootWidget)
	UE_API void RequestPrimaryControl(UGameRootLayoutWidget* Layout);

protected:
	UE_API void AddLayoutToViewport(ULocalPlayer* LocalPlayer, UGameRootLayoutWidget* Layout);
	UE_API void RemoveLayoutFromViewport(ULocalPlayer* LocalPlayer, UGameRootLayoutWidget* Layout);

	UE_API virtual void OnRootLayoutAddedToViewport(ULocalPlayer* LocalPlayer, UGameRootLayoutWidget* Layout);
	UE_API virtual void OnRootLayoutRemovedFromViewport(ULocalPlayer* LocalPlayer, UGameRootLayoutWidget* Layout);
	UE_API virtual void OnRootLayoutReleased(ULocalPlayer* LocalPlayer, UGameRootLayoutWidget* Layout);

	UE_API void CreateLayoutWidget(ULocalPlayer* LocalPlayer);
	UE_API TSubclassOf<UGameRootLayoutWidget> GetLayoutWidgetClass();

private:
	ELocalMultiplayerViewMode LocalMultiplayerInteractionMode = ELocalMultiplayerViewMode::PrimaryOnly;

	// 此 UI 策略 使用的 RootWidget 软引用,全部 LocalPlayer 都会使用
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UGameRootLayoutWidget> LayoutClass;

	UPROPERTY(Transient)
	TArray<FRootViewportLayoutInfo> RootViewportLayouts;

	// 外部传递的事件流 GameInstance->UIManager->this , 通知本地玩家的增删
	UE_API void NotifyPlayerAdded(ULocalPlayer* LocalPlayer);
	UE_API void NotifyPlayerRemoved(ULocalPlayer* LocalPlayer);
	UE_API void NotifyPlayerDestroyed(ULocalPlayer* LocalPlayer);

	friend class UUIManager;
};

#undef UE_API
