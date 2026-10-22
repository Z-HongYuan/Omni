// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/LocalPlayer.h"
#include "ExtLocalPlayer.generated.h"

#define UE_API COREEXTENSION_API

class UGameUIRootWidget;

/** 保存本地玩家视图开关，并转发 Controller、PlayerState 和 Pawn 的就绪通知。 */
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

	/** Controller 已存在时立即调用，再注册后续变化通知；调用方负责移除返回的委托句柄。 */
	UE_API FDelegateHandle CallOrRegister_OnPlayerControllerSet(const FPlayerControllerSetDelegate::FDelegate& Delegate);
	/** PlayerState 已存在时立即调用，再注册后续变化通知；调用方负责移除返回的委托句柄。 */
	UE_API FDelegateHandle CallOrRegister_OnPlayerStateSet(const FPlayerStateSetDelegate::FDelegate& Delegate);
	/** Pawn 已存在时立即调用，再注册后续变化通知；调用方负责移除返回的委托句柄。 */
	UE_API FDelegateHandle CallOrRegister_OnPlayerPawnSet(const FPlayerPawnSetDelegate::FDelegate& Delegate);

	UE_API virtual bool GetProjectionData(FViewport* Viewport, FSceneViewProjectionData& ProjectionData, int32 StereoViewIndex) const override;

	bool IsPlayerViewEnabled() const { return bIsPlayerViewEnabled; }
	void SetIsPlayerViewEnabled(bool bInIsPlayerViewEnabled) { bIsPlayerViewEnabled = bInIsPlayerViewEnabled; }

private:
	bool bIsPlayerViewEnabled = true;
};

#undef UE_API
