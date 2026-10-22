// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameUIManager.generated.h"

#define UE_API GAMEUI_API

class ULocalPlayer;
class UGameUIPolicy;

/** 管理当前 UI 策略，转发本地玩家事件并同步根布局可见性。 */
UCLASS(MinimalAPI, Config = Game)
class UGameUIManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UGameUIManager() { ; }

	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;
	UE_API virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	const UGameUIPolicy* GetCurrentUIPolicy() const { return CurrentPolicy; }
	UGameUIPolicy* GetCurrentUIPolicy() { return CurrentPolicy; }

	/** 通知当前策略创建或恢复本地玩家的根布局。 */
	UE_API virtual void NotifyPlayerAdded(ULocalPlayer* LocalPlayer);
	/** 通知当前策略暂时移除本地玩家的根布局，保留布局记录。 */
	UE_API virtual void NotifyPlayerRemoved(ULocalPlayer* LocalPlayer);
	/** 通知当前策略释放本地玩家的布局记录和事件绑定。 */
	UE_API virtual void NotifyPlayerDestroyed(ULocalPlayer* LocalPlayer);

protected:
	UE_API void SwitchToPolicy(UGameUIPolicy* InPolicy);

private:
	UPROPERTY(Transient)
	TObjectPtr<UGameUIPolicy> CurrentPolicy = nullptr;

	/** 每帧根据玩家 HUD 的显示开关同步根布局可见性。 */
	bool Tick(float DeltaTime);
	void SyncRootLayoutVisibilityToShowHUD();
	FTSTicker::FDelegateHandle TickHandle;
};

#undef UE_API
