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

/** 连接用户、会话和消息系统，转发游戏实例级流程。 */
UCLASS(MinimalAPI, Abstract, Config = Game)
class UExtGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UE_API UExtGameInstance(const FObjectInitializer& ObjectInitializer);

	UE_API virtual void Init() override;
	UE_API virtual void ReturnToMainMenu() override;

	/** 将 GameUser 错误转发给首个本地玩家的消息对话框。 */
	UFUNCTION()
	UE_API virtual void HandleSystemMessage(FGameplayTag MessageType, FText Title, FText Message);
	UFUNCTION()
	UE_API virtual void HandlePrivilegeChanged(const UGameUserInfo* UserInfo, EGameUserPrivilege Privilege, EGameUserAvailability OldAvailability, EGameUserAvailability NewAvailability);
	UFUNCTION()
	UE_API virtual void HandlerUserInitialized(const UGameUserInfo* UserInfo, bool bSuccess, FText Error, EGameUserPrivilege RequestedPrivilege, EGameUserOnlineContext OnlineContext);


	/** 重置 GameUser 状态并清理会话，供返回主菜单等流程调用。 */
	UE_API virtual void ResetUserAndSessionState();

	// 外部邀请经 SetRequestedSession 缓存；无法立即加入时先切换到可加入会话的状态。

	/** 获取尚待处理的会话请求。 */
	UGameUserSession_SearchResult* GetRequestedSession() const { return RequestedSession; }

	/** 处理用户从外部源（例如平台覆盖层）接受会话邀请。预期由每个游戏重写。 */
	UE_API virtual void OnUserRequestedSession(const FPlatformUserId& PlatformUserId, UGameUserSession_SearchResult* InRequestedSession, const FOnlineResultInformation& RequestedSessionResult);

	/** 处理 OSS 的会话销毁请求，并返回主菜单。 */
	UE_API virtual void OnDestroySessionRequested(const FPlatformUserId& PlatformUserId, const FName& SessionName);

	/** 设置（或清除）请求的会话。设置后，请求会话流程开始。 */
	UE_API virtual void SetRequestedSession(UGameUserSession_SearchResult* InRequestedSession);

	/** 检查是否可以加入请求的会话。可由每个游戏重写。 */
	UE_API virtual bool CanJoinRequestedSession() const;

	/** 使用首个本地玩家加入缓存的会话；提交前清除缓存。 */
	UE_API virtual void JoinRequestedSession();

	/** 默认返回主菜单；派生类在游戏就绪后应调用 JoinRequestedSession。 */
	UE_API virtual void ResetGameAndJoinRequestedSession();

private:
	UPROPERTY()
	TObjectPtr<UGameUserSession_SearchResult> RequestedSession;
};

#undef UE_API
