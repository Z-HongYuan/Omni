// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UIManager.generated.h"

#define UE_API GAMEUI_API

class ULocalPlayer;
class UUIPolicy;

/**
 * 针对于 LocalPlayer 和 UIPolicy 的管理
 */
UCLASS(MinimalAPI, Config = Game)
class UUIManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UUIManager() { ; }

	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;
	UE_API virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	const UUIPolicy* GetCurrentUIPolicy() const { return CurrentPolicy; }
	UUIPolicy* GetCurrentUIPolicy() { return CurrentPolicy; }

	// 由外部调用的事件流, 设置为从 GameInstance 中传递
	UE_API virtual void NotifyPlayerAdded(ULocalPlayer* LocalPlayer);
	// 由外部调用的事件流, 设置为从 GameInstance 中传递
	UE_API virtual void NotifyPlayerRemoved(ULocalPlayer* LocalPlayer);
	// 由外部调用的事件流, 设置为从 GameInstance 中传递
	UE_API virtual void NotifyPlayerDestroyed(ULocalPlayer* LocalPlayer);

protected:
	UE_API void SwitchToPolicy(UUIPolicy* InPolicy);

private:
	UPROPERTY(Transient)
	TObjectPtr<UUIPolicy> CurrentPolicy = nullptr;

	// 对于不会显示HUD的情况下,设置根控件的可视性
	bool Tick(float DeltaTime);
	void SyncRootLayoutVisibilityToShowHUD();
	FTSTicker::FDelegateHandle TickHandle;
};

#undef UE_API
