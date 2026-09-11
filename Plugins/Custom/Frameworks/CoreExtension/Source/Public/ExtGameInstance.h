// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/GameInstance.h"
#include "ExtGameInstance.generated.h"

#define UE_API COREEXTENSION_API

struct FOnlineResultInformation;
class UGameUserSession_SearchResult;
enum class EGameUserOnlineContext : uint8;
enum class EGameUserAvailability : uint8;
enum class EGameUserPrivilege : uint8;
class UGameUserInfo;
struct FGameplayTag;

/**
 * 自定义的游戏实例,充当胶水模块
 */
UCLASS(MinimalAPI, Abstract, Config = Game)
class UExtGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UE_API UExtGameInstance(const FObjectInitializer& ObjectInitializer);

	UE_API virtual void Init() override;
	UE_API virtual void ReturnToMainMenu() override;

	/** 处理 GameUser 的错误 使用 MessagingSystem 进行 UI 展示*/
	UFUNCTION()
	UE_API virtual void HandleSystemMessage(FGameplayTag MessageType, FText Title, FText Message);
	UFUNCTION()
	UE_API virtual void HandlePrivilegeChanged(const UGameUserInfo* UserInfo, EGameUserPrivilege Privilege, EGameUserAvailability OldAvailability, EGameUserAvailability NewAvailability);
	UFUNCTION()
	UE_API virtual void HandlerUserInitialized(const UGameUserInfo* UserInfo, bool bSuccess, FText Error, EGameUserPrivilege RequestedPrivilege, EGameUserOnlineContext OnlineContext);


	/** 调用重置用户和会话状态，通常是因为某位玩家已经断开连接 传递到 GameUser 和 SessionSubsystem */
	UE_API virtual void ResetUserAndSessionState();

	/**
	 * 请求会话流程
	 *   某处请求用户加入特定会话（例如，通过平台覆盖层触发 OnUserRequestedSession）。
	 *   此请求在 SetRequestedSession 中处理。
	 *   检查是否可以立即加入请求的会话（CanJoinRequestedSession）。如果可以，加入该会话（JoinRequestedSession）。
	 *   如果不行，缓存请求的会话并指示游戏进入可以加入会话的状态（ResetGameAndJoinRequestedSession）。
	 */

	/** 获取缓存的会话 */
	UGameUserSession_SearchResult* GetRequestedSession() const { return RequestedSession; }

	/** 处理用户从外部源（例如平台覆盖层）接受会话邀请。预期由每个游戏重写。 */
	UE_API virtual void OnUserRequestedSession(const FPlatformUserId& PlatformUserId, UGameUserSession_SearchResult* InRequestedSession, const FOnlineResultInformation& RequestedSessionResult);

	/** 处理 OSS 请求销毁会话 */
	UE_API virtual void OnDestroySessionRequested(const FPlatformUserId& PlatformUserId, const FName& SessionName);

	/** 设置（或清除）请求的会话。设置后，请求会话流程开始。 */
	UE_API virtual void SetRequestedSession(UGameUserSession_SearchResult* InRequestedSession);

	/** 检查是否可以加入请求的会话。可由每个游戏重写。 */
	UE_API virtual bool CanJoinRequestedSession() const;

	/** 加入请求的会话 */
	UE_API virtual void JoinRequestedSession();

	/** 使游戏进入可以加入请求会话的状态 */
	UE_API virtual void ResetGameAndJoinRequestedSession();

private:
	UPROPERTY()
	TObjectPtr<UGameUserSession_SearchResult> RequestedSession;
};

#undef UE_API
