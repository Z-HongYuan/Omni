// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/LocalPlayer.h"
#include "ExtensionLocalPlayer.generated.h"

#define UE_API GAMECOREEXTENSION_API

class UGameRootLayoutWidget;

/**
 * 自定义的本地玩家,承载游戏流程的转发
 */
UCLASS(MinimalAPI, Transient, Config = Game)
class UExtensionLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()

public:
	UE_API UExtensionLocalPlayer(const FObjectInitializer& ObjectInitializer);

	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerControllerSetDelegate, UExtensionLocalPlayer* LocalPlayer, APlayerController* PlayerController);
	FPlayerControllerSetDelegate OnPlayerControllerSet;

	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerStateSetDelegate, UExtensionLocalPlayer* LocalPlayer, APlayerState* PlayerState);
	FPlayerStateSetDelegate OnPlayerStateSet;

	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerPawnSetDelegate, UExtensionLocalPlayer* LocalPlayer, APawn* Pawn);
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
