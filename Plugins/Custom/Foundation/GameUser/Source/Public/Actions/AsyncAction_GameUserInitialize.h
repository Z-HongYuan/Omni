// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/CancellableAsyncAction.h"
#include "System/GameUserSubsystem.h"
#include "AsyncAction_GameUserInitialize.generated.h"

#define UE_API GAMEUSER_API

/** 包装本地与在线登录请求，在初始化完成后广播结果并释放异步节点。 */
UCLASS(MinimalAPI)
class UAsyncAction_GameUserInitialize : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	/**
	 * 创建本地玩家初始化任务，包含平台登录和 CanPlay 权限检查。
	 * LocalPlayerIndex 为目标本地玩家索引；无效输入设备使用系统默认设备。
	 * bCanUseGuestLogin 允许附加玩家使用访客身份，完成后广播结果。
	 */
	UFUNCTION(BlueprintCallable, Category = GameUser, meta = (BlueprintInternalUseOnly = "true"))
	static UE_API UAsyncAction_GameUserInitialize* InitializeForLocalPlay(UGameUserSubsystem* Target, int32 LocalPlayerIndex, FInputDeviceId PrimaryInputDevice, bool bCanUseGuestLogin);

	/** 为指定索引的现有本地玩家创建在线登录任务，完成后广播结果。 */
	UFUNCTION(BlueprintCallable, Category = GameUser, meta = (BlueprintInternalUseOnly = "true"))
	static UE_API UAsyncAction_GameUserInitialize* LoginForOnlinePlay(UGameUserSubsystem* Target, int32 LocalPlayerIndex);

	/** 初始化成功或失败时调用。 */
	UPROPERTY(BlueprintAssignable)
	FGameUserOnInitializeCompleteMulticast OnInitializationComplete;

	UE_API virtual void Activate() override;

	/** 将启动失败转换为初始化完成回调；节点已取消时不再广播。 */
	UE_API void HandleFailure();

	/** 节点仍允许广播时转发初始化结果，随后结束节点生命周期。 */
	UFUNCTION()
	UE_API virtual void HandleInitializationComplete(const UGameUserInfo* UserInfo, bool bSuccess, FText Error, EGameUserPrivilege RequestedPrivilege, EGameUserOnlineContext OnlineContext);

protected:
	/** 处理登录请求的子系统弱引用，不延长其生命周期。 */
	TWeakObjectPtr<UGameUserSubsystem> Subsystem;
	FGameUserInitializeParams Params;
};

#undef UE_API
