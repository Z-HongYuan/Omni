// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/LocalPlayer.h"
#include "ExtLocalPlayer.generated.h"

#define UE_API COREEXTENSION_API

class UGameUIRootWidget;

/**
 * 自定义的本地玩家,承载游戏流程的转发
 */
UCLASS(MinimalAPI, Transient, Config = Game)
class UExtLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()

public:
	UE_API UExtLocalPlayer(const FObjectInitializer& ObjectInitializer);

	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerControllerSetDelegate, UExtLocalPlayer* LocalPlayer, APlayerController* PlayerController);
	FPlayerControllerSetDelegate OnPlayerControllerSet;

	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerStateSetDelegate, UExtLocalPlayer* LocalPlayer, APlayerState* PlayerState);
	FPlayerStateSetDelegate OnPlayerStateSet;

	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerPawnSetDelegate, UExtLocalPlayer* LocalPlayer, APawn* Pawn);
	FPlayerPawnSetDelegate OnPlayerPawnSet;

	UE_API FDelegateHandle CallOrRegister_OnPlayerControllerSet(const FPlayerControllerSetDelegate::FDelegate& Delegate);
	UE_API FDelegateHandle CallOrRegister_OnPlayerStateSet(const FPlayerStateSetDelegate::FDelegate& Delegate);
	UE_API FDelegateHandle CallOrRegister_OnPlayerPawnSet(const FPlayerPawnSetDelegate::FDelegate& Delegate);

	UE_API virtual bool GetProjectionData(FViewport* Viewport, FSceneViewProjectionData& ProjectionData, int32 StereoViewIndex) const override;

	bool IsPlayerViewEnabled() const { return bIsPlayerViewEnabled; }
	void SetIsPlayerViewEnabled(bool bInIsPlayerViewEnabled) { bIsPlayerViewEnabled = bInIsPlayerViewEnabled; }

private:
	bool bIsPlayerViewEnabled = true;
};

#undef UE_API
