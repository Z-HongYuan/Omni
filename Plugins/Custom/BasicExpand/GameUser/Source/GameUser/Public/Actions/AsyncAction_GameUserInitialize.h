// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/CancellableAsyncAction.h"
#include "System/GameUserSubsystem.h"
#include "AsyncAction_GameUserInitialize.generated.h"

#define UE_API GAMEUSER_API

/**
 * 用于处理初始化用户的各种功能的异步操作
 */
UCLASS(MinimalAPI)
class UAsyncAction_GameUserInitialize : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	/**
	 * 使用通用用户系统初始化本地玩家,包括执行平台特定的登录和权限检查。
	 * 当过程成功或失败时,将广播 OnInitializationComplete 委托。
	 *
	 * @param LocalPlayerIndex	ULocalPlayer 在游戏实例中的期望索引,0 为主玩家,1 及以上用于本地多人游戏
	 * @param PrimaryInputDevice 用户的主要输入设备,如果无效则使用系统默认值
	 * @param bCanUseGuestLogin	如果为 true,则此玩家可以在没有真实系统网络 ID 的情况下作为访客
	 */
	UFUNCTION(BlueprintCallable, Category = GameUser, meta = (BlueprintInternalUseOnly = "true"))
	static UE_API UAsyncAction_GameUserInitialize* InitializeForLocalPlay(UGameUserSubsystem* Target, int32 LocalPlayerIndex, FInputDeviceId PrimaryInputDevice, bool bCanUseGuestLogin);

	/**
	 * 尝试将现有用户登录到平台特定的在线后端,以启用完整的在线游戏功能
	 * 当过程成功或失败时,将广播 OnInitializationComplete 委托。
	 *
	 * @param LocalPlayerIndex	现有 LocalPlayer 在游戏实例中的索引
	 */
	UFUNCTION(BlueprintCallable, Category = GameUser, meta = (BlueprintInternalUseOnly = "true"))
	static UE_API UAsyncAction_GameUserInitialize* LoginForOnlinePlay(UGameUserSubsystem* Target, int32 LocalPlayerIndex);

	/** 初始化成功或失败时调用 */
	UPROPERTY(BlueprintAssignable)
	FGameUserOnInitializeCompleteMulticast OnInitializationComplete;

	UE_API virtual void Activate() override;

	/** 失败并按需发送回调 */
	UE_API void HandleFailure();

	/** 包装委托,如合适将传递给 OnInitializationComplete */
	UFUNCTION()
	UE_API virtual void HandleInitializationComplete(const UGameUserInfo* UserInfo, bool bSuccess, FText Error, EGameUserPrivilege RequestedPrivilege, EGameUserOnlineContext OnlineContext);

protected:
	/** 实际启动初始化 */

	TWeakObjectPtr<UGameUserSubsystem> Subsystem;
	FGameUserInitializeParams Params;
};

#undef UE_API
