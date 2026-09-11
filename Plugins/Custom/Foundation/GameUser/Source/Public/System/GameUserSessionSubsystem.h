// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "PartyBeaconClient.h"
#include "PartyBeaconHost.h"
#include "PartyBeaconState.h"
#include "Data/GameUserSession_HostSessionRequest.h"
#include "Data/GameUserSession_SearchResult.h"
#include "Data/GameUserSession_SearchSessionRequest.h"
#include "Data/GameUserTypes.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/ObjectPtr.h"
#include "UObject/PrimaryAssetId.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/WeakObjectPtr.h"
#include "GameUserSessionSubsystem.generated.h"

#define UE_API GAMEUSER_API

class FGameOnlineSearchSettingsOSSv1;

/**
 * 当会话加入完成时触发的事件,发生在加入底层会话之后、如果成功则跳转到服务器之前。
 * 事件参数指示是否成功,或是否存在将阻止其跳转的错误。
 * @param Result 会话加入的结果
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FGameUserSessionOnJoinSessionComplete, const FOnlineResultInformation& /*Result*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGameUserSessionOnJoinSessionComplete_Dynamic, const FOnlineResultInformation&, Result);

/**
 * 当用于托管的会话创建完成时触发的事件,正好在其跳转到地图之前。
 * 事件参数指示是否成功,或是否存在将阻止其跳转的错误。
 * @param Result 会话加入的结果
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FGameUserSessionOnCreateSessionComplete, const FOnlineResultInformation& /*Result*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGameUserSessionOnCreateSessionComplete_Dynamic, const FOnlineResultInformation&, Result);

/**
 * 当本地用户从外部来源(例如平台覆盖层)请求销毁会话时触发的事件。
 * 游戏应将玩家从该会话中移出。
 * @param LocalPlatformUserId 发出销毁请求的本地用户 ID。由于用户可能尚未登录,因此这是平台用户 ID。
 * @param SessionName 会话的名称标识符。
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FGameUserSessionOnDestroySessionRequested, const FPlatformUserId& /*LocalPlatformUserId*/, const FName& /*SessionName*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGameUserSessionOnDestroySessionRequested_Dynamic, const FPlatformUserId&, LocalPlatformUserId, const FName&, SessionName);

/**
 * 会话加入完成后触发的事件,发生在解析连接字符串之后、客户端跳转之前。
 * @param URL 已解析的会话连接字符串,包含任何附加参数
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FGameUserSessionOnPreClientTravel, FString& /*URL*/);

/**
 * 在会话生态系统的不同时间点触发的事件,用于表示会话的可展示给用户的状态。
 * 不应将其用于在线功能(这些功能请使用 OnCreateSessionComplete 或 OnJoinSessionComplete),而应用于丰富在线状态等功能
 */
UENUM(BlueprintType)
enum class EGameUserSessionInformationState : uint8
{
	OutOfGame,
	Matchmaking,
	InGame
};

DECLARE_MULTICAST_DELEGATE_ThreeParams(FGameUserSessionOnSessionInformationChanged, EGameUserSessionInformationState /*SessionStatus*/, const FString& /*GameMode*/, const FString& /*MapName*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FGameUserSessionOnSessionInformationChanged_Dynamic, EGameUserSessionInformationState, SessionStatus, const FString&, GameMode, const FString&, MapName);


/** 
 * 处理在线游戏托管和加入请求的游戏子系统。
 * 每个游戏实例会创建一个子系统,可从蓝图或 C++ 代码访问。
 * 如果存在游戏特定的子类,则不会创建此基础子系统。
 */
UCLASS(MinimalAPI, BlueprintType, Config=Engine)
class UGameUserSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;
	UE_API virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	/** 使用在线游戏的默认选项创建托管会话请求,创建后可修改 */
	UFUNCTION(BlueprintCallable, Category = Session)
	UE_API virtual UGameUserSession_HostSessionRequest* CreateOnlineHostSessionRequest();

	/** 使用默认选项创建会话搜索对象以查找默认在线游戏,创建后可修改 */
	UFUNCTION(BlueprintCallable, Category = Session)
	UE_API virtual UGameUserSession_SearchSessionRequest* CreateOnlineSearchSessionRequest();

	/** 使用会话请求信息创建新的在线游戏,如果成功将开始硬地图切换 */
	UFUNCTION(BlueprintCallable, Category=Session)
	UE_API virtual void HostSession(APlayerController* HostingPlayer, UGameUserSession_HostSessionRequest* Request);

	/** 启动查找现有会话的过程,如果没有找到可行会话则创建新会话 */
	UFUNCTION(BlueprintCallable, Category=Session)
	UE_API virtual void QuickPlaySession(APlayerController* JoiningOrHostingPlayer, UGameUserSession_HostSessionRequest* Request);

	/** 启动加入现有会话的过程,如果成功将连接到指定的服务器 */
	UFUNCTION(BlueprintCallable, Category=Session)
	UE_API virtual void JoinSession(APlayerController* JoiningPlayer, UGameUserSession_SearchResult* Request);

	/** 向在线系统查询与搜索请求匹配的可加入会话列表 */
	UFUNCTION(BlueprintCallable, Category=Session)
	UE_API virtual void FindSessions(APlayerController* SearchingPlayer, UGameUserSession_SearchSessionRequest* Request);

	/** 清理所有活动会话,在返回主菜单等情况下调用 */
	UFUNCTION(BlueprintCallable, Category=Session)
	UE_API virtual void CleanUpSessions();

	//////////////////////////////////////////////////////////////////////
	// 事件

	/** 本地用户接受邀请时的原生委托 */
	FGameUserSessionOnUserRequestedSession OnUserRequestedSessionEvent;
	/** 本地用户接受邀请时广播的事件 */
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On User Requested Session"))
	FGameUserSessionOnUserRequestedSession_Dynamic K2_OnUserRequestedSessionEvent;

	/** JoinSession 调用完成时的原生委托 */
	FGameUserSessionOnJoinSessionComplete OnJoinSessionCompleteEvent;
	/** JoinSession 调用完成时广播的事件 */
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On Join Session Complete"))
	FGameUserSessionOnJoinSessionComplete_Dynamic K2_OnJoinSessionCompleteEvent;

	/** CreateSession 调用完成时的原生委托 */
	FGameUserSessionOnCreateSessionComplete OnCreateSessionCompleteEvent;
	/** CreateSession 调用完成时广播的事件 */
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On Create Session Complete"))
	FGameUserSessionOnCreateSessionComplete_Dynamic K2_OnCreateSessionCompleteEvent;

	/** 可展示的会话信息发生变化时的原生委托 */
	FGameUserSessionOnSessionInformationChanged OnSessionInformationChangedEvent;
	/** 可展示的会话信息发生变化时广播的事件 */
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On Session Information Changed"))
	FGameUserSessionOnSessionInformationChanged_Dynamic K2_OnSessionInformationChangedEvent;

	/** 已请求销毁平台会话时的原生委托 */
	FGameUserSessionOnDestroySessionRequested OnDestroySessionRequestedEvent;
	/** 已请求销毁平台会话时广播的事件 */
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On Leave Session Requested"))
	FGameUserSessionOnDestroySessionRequested_Dynamic K2_OnDestroySessionRequestedEvent;

	/** 在客户端跳转前修改连接 URL 的原生委托 */
	FGameUserSessionOnPreClientTravel OnPreClientTravelEvent;

	// 配置设置,可在子类或配置文件中覆盖

	/** 为会话搜索和托管请求设置 bUseLobbies 的默认值 */
	UPROPERTY(Config)
	bool bUseLobbiesDefault = true;

	/** 为会话托管请求设置 bUseLobbiesVoiceChat 的默认值 */
	UPROPERTY(Config)
	bool bUseLobbiesVoiceChatDefault = false;

	/** 在创建或加入游戏会话时,于服务器跳转前启用预留信标流程 */
	UPROPERTY(Config)
	bool bUseBeacons = true;

protected:
	// 在创建或加入会话过程中调用的函数,可针对游戏特定行为进行覆盖

	/** 用于根据快速开始托管设置填充会话请求,可针对游戏特定行为进行覆盖 */
	UE_API virtual TSharedRef<FGameOnlineSearchSettingsOSSv1> CreateQuickPlaySearchSettings(UGameUserSession_HostSessionRequest* Request, UGameUserSession_SearchSessionRequest* QuickPlayRequest);

	/** 快速开始搜索完成时调用,可针对游戏特定行为进行覆盖 */
	UE_API virtual void HandleQuickPlaySearchFinished(bool bSucceeded, const FText& ErrorMessage, TWeakObjectPtr<APlayerController> JoiningOrHostingPlayer, TStrongObjectPtr<UGameUserSession_HostSessionRequest> HostRequest);

	/** 跳转到会话失败时调用 */
	UE_API virtual void TravelLocalSessionFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ReasonString);

	/** 新会话创建成功或创建失败时调用 */
	UE_API virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	/** 用于完成会话创建 */
	UE_API virtual void FinishSessionCreation(bool bWasSuccessful);

	/** 跳转到新的托管会话地图后调用 */
	UE_API virtual void HandlePostLoadMap(UWorld* World);

protected:
	// 用于初始化和处理来自在线系统的结果的内部函数

	UE_API void BindOnlineDelegates();
	UE_API void CreateOnlineSessionInternal(ULocalPlayer* LocalPlayer, UGameUserSession_HostSessionRequest* Request);
	UE_API void FindSessionsInternal(APlayerController* SearchingPlayer, const TSharedRef<FGameOnlineSearchSettingsOSSv1>& InSearchSettings);
	UE_API void JoinSessionInternal(ULocalPlayer* LocalPlayer, UGameUserSession_SearchResult* Request);
	UE_API void InternalTravelToSession(const FName SessionName);
	UE_API void NotifyUserRequestedSession(const FPlatformUserId& PlatformUserId, UGameUserSession_SearchResult* RequestedSession, const FOnlineResultInformation& RequestedSessionResult);
	UE_API void NotifyJoinSessionComplete(const FOnlineResultInformation& Result);
	UE_API void NotifyCreateSessionComplete(const FOnlineResultInformation& Result);
	UE_API void NotifySessionInformationUpdated(EGameUserSessionInformationState SessionStatusStr, const FString& GameMode = FString(), const FString& MapName = FString());
	UE_API void NotifyDestroySessionRequested(const FPlatformUserId& PlatformUserId, const FName& SessionName);
	UE_API void SetCreateSessionError(const FText& ErrorText);

	UE_API void BindOnlineDelegatesOSSv1();
	UE_API void CreateOnlineSessionInternalOSSv1(ULocalPlayer* LocalPlayer, UGameUserSession_HostSessionRequest* Request);
	UE_API void FindSessionsInternalOSSv1(ULocalPlayer* LocalPlayer);
	UE_API void JoinSessionInternalOSSv1(ULocalPlayer* LocalPlayer, UGameUserSession_SearchResult* Request);
	UE_API TSharedRef<FGameOnlineSearchSettingsOSSv1> CreateQuickPlaySearchSettingsOSSv1(UGameUserSession_HostSessionRequest* Request, UGameUserSession_SearchSessionRequest* QuickPlayRequest);
	UE_API void CleanUpSessionsOSSv1();

	UE_API void HandleSessionFailure(const FUniqueNetId& NetId, ESessionFailure::Type FailureType);
	UE_API void HandleSessionUserInviteAccepted(const bool bWasSuccessful, const int32 LocalUserIndex, FUniqueNetIdPtr AcceptingUserId, const FOnlineSessionSearchResult& SearchResult);
	UE_API void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	UE_API void OnRegisterLocalPlayerComplete_CreateSession(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result);
	UE_API void OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful);
	UE_API void OnEndSessionComplete(FName SessionName, bool bWasSuccessful);
	UE_API void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	UE_API void OnDestroySessionRequested(int32 LocalUserNum, FName SessionName);
	UE_API void OnFindSessionsComplete(bool bWasSuccessful);
	UE_API void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	UE_API void OnRegisterJoiningLocalPlayerComplete(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result);
	UE_API void FinishJoinSession(EOnJoinSessionCompleteResult::Type Result);


	UE_API void CreateHostReservationBeacon();
	UE_API void ConnectToHostReservationBeacon();
	UE_API void DestroyHostReservationBeacon();

protected:
	/** 会话操作完成后将使用的跳转 URL */
	FString PendingTravelURL;

	/** 最近一次会话创建尝试的结果信息,存储在此处以便后续保留错误代码 */
	FOnlineResultInformation CreateSessionResult;

	/** 如果我们希望在会话创建后取消该会话,则为 true */
	bool bWantToDestroyPendingSession = false;

	/** 如果这是专用服务器,则为 true,专用服务器无需 LocalPlayer 即可创建会话 */
	bool bIsDedicatedServer = false;

	/** 当前搜索的设置 */
	TSharedPtr<FGameOnlineSearchSettingsOSSv1> SearchSettings;

	/** 用于注册信标的通用信标侦听器 */
	UPROPERTY(Transient)
	TWeakObjectPtr<AOnlineBeaconHost> BeaconHostListener;
	/** 信标主机的状态 */
	UPROPERTY(Transient)
	TObjectPtr<UPartyBeaconState> ReservationBeaconHostState;
	/** 控制本游戏访问权限的信标。 */
	UPROPERTY(Transient)
	TWeakObjectPtr<APartyBeaconHost> ReservationBeaconHost;
	/** 用于信标通信的通用类对象 */
	UPROPERTY(Transient)
	TWeakObjectPtr<APartyBeaconClient> ReservationBeaconClient;

	/** 信标预留的团队数量 */
	UPROPERTY(Config)
	int32 BeaconTeamCount = 2;
	/** 信标预留的团队大小 */
	UPROPERTY(Config)
	int32 BeaconTeamSize = 8;
	/** 信标预留的最大数量 */
	UPROPERTY(Config)
	int32 BeaconMaxReservations = 16;
};

#undef UE_API
