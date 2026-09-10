// Copyright © 2026 张鸿源. All Rights Reserved.

#include "System/GameUserSessionSubsystem.h"

#include "LogGameUser.h"
#include "OnlineBeaconHost.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "AssetRegistry/AssetData.h"
#include "Data/GameUserSessionSearchSettings.h"
#include "Data/GameUserTypes.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/OnlineSessionDelegates.h"
#include "Misc/ConfigCacheIni.h"
#include "Online/OnlineSessionNames.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameUserSessionSubsystem)

#define LOCTEXT_NAMESPACE "GameUser"

void UGameUserSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	BindOnlineDelegates();
	GEngine->OnTravelFailure().AddUObject(this, &UGameUserSessionSubsystem::TravelLocalSessionFailure);

	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UGameUserSessionSubsystem::HandlePostLoadMap);

	UGameInstance* GameInstance = GetGameInstance();
	bIsDedicatedServer = GameInstance->IsDedicatedServerInstance();
}

void UGameUserSessionSubsystem::Deinitialize()
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());

	if (OnlineSub)
	{
		// 在关闭期间,这可能无效
		const IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
		if (SessionInterface)
		{
			SessionInterface->ClearOnSessionFailureDelegates(this);
		}
	}

	if (GEngine)
	{
		GEngine->OnTravelFailure().RemoveAll(this);
	}

	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

	Super::Deinitialize();
}

bool UGameUserSessionSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	TArray<UClass*> ChildClasses;
	GetDerivedClasses(GetClass(), ChildClasses, false);

	// 仅当不存在游戏专属子类时才创建实例
	return ChildClasses.Num() == 0;
}


UGameUserSession_HostSessionRequest* UGameUserSessionSubsystem::CreateOnlineHostSessionRequest()
{
	/** 游戏专属子系统可以覆盖此方法,或者你可以在创建后自行修改 */

	UGameUserSession_HostSessionRequest* NewRequest = NewObject<UGameUserSession_HostSessionRequest>(this);
	NewRequest->OnlineMode = EGameUserSessionOnlineMode::Online;
	NewRequest->bUseLobbies = bUseLobbiesDefault;
	NewRequest->bUseLobbiesVoiceChat = bUseLobbiesVoiceChatDefault;

	// 默认在用于匹配的主会话中启用在线状态。对于关心在线状态的在线系统,只有主会话应启用在线状态
	NewRequest->bUsePresence = !IsRunningDedicatedServer();

	return NewRequest;
}

UGameUserSession_SearchSessionRequest* UGameUserSessionSubsystem::CreateOnlineSearchSessionRequest()
{
	/** 游戏专属子系统可以覆盖此方法,或者你可以在创建后自行修改 */

	UGameUserSession_SearchSessionRequest* NewRequest = NewObject<UGameUserSession_SearchSessionRequest>(this);
	NewRequest->OnlineMode = EGameUserSessionOnlineMode::Online;

	NewRequest->bUseLobbies = bUseLobbiesDefault;

	return NewRequest;
}

void UGameUserSessionSubsystem::HostSession(APlayerController* HostingPlayer, UGameUserSession_HostSessionRequest* Request)
{
	if (Request == nullptr)
	{
		SetCreateSessionError(NSLOCTEXT("NetworkErrors", "InvalidRequest", "HostSession passed an invalid request."));
		OnCreateSessionComplete(NAME_None, false);
		return;
	}

	ULocalPlayer* LocalPlayer = (HostingPlayer != nullptr) ? HostingPlayer->GetLocalPlayer() : nullptr;
	if (LocalPlayer == nullptr && !bIsDedicatedServer)
	{
		SetCreateSessionError(NSLOCTEXT("NetworkErrors", "InvalidHostingPlayer", "HostingPlayer is invalid."));
		OnCreateSessionComplete(NAME_None, false);
		return;
	}

	FText OutError;
	if (!Request->ValidateAndLogErrors(OutError))
	{
		SetCreateSessionError(OutError);
		OnCreateSessionComplete(NAME_None, false);
		return;
	}

	if (Request->OnlineMode == EGameUserSessionOnlineMode::Offline)
	{
		if (GetWorld()->GetNetMode() == NM_Client)
		{
			SetCreateSessionError(NSLOCTEXT("NetworkErrors", "CannotHostAsClient", "Cannot host offline game as client."));
			OnCreateSessionComplete(NAME_None, false);
			return;
		}
		else
		{
			// 离线模式,因此立即跳转到指定的比赛 URL
			GetWorld()->ServerTravel(Request->ConstructTravelURL());
		}
	}
	else
	{
		CreateOnlineSessionInternal(LocalPlayer, Request);
	}

	NotifySessionInformationUpdated(EGameUserSessionInformationState::InGame, Request->ModeNameForAdvertisement, Request->GetMapName());
}

void UGameUserSessionSubsystem::QuickPlaySession(APlayerController* JoiningOrHostingPlayer, UGameUserSession_HostSessionRequest* HostRequest)
{
	UE_LOG(LogGameUserSession, Log, TEXT("QuickPlay Requested"));

	if (HostRequest == nullptr)
	{
		UE_LOG(LogGameUserSession, Error, TEXT("QuickPlaySession passed a null request"));
		return;
	}

	TStrongObjectPtr<UGameUserSession_HostSessionRequest> HostRequestPtr = TStrongObjectPtr<UGameUserSession_HostSessionRequest>(HostRequest);
	TWeakObjectPtr<APlayerController> JoiningOrHostingPlayerPtr = TWeakObjectPtr<APlayerController>(JoiningOrHostingPlayer);

	UGameUserSession_SearchSessionRequest* QuickPlayRequest = CreateOnlineSearchSessionRequest();
	QuickPlayRequest->OnSearchFinished.AddUObject(this, &UGameUserSessionSubsystem::HandleQuickPlaySearchFinished, JoiningOrHostingPlayerPtr, HostRequestPtr);

	// 默认在用于匹配的主会话上启用在线状态。对于关心在线状态的在线系统,只有主会话应启用在线状态

	HostRequestPtr->bUseLobbies = bUseLobbiesDefault;
	HostRequestPtr->bUseLobbiesVoiceChat = bUseLobbiesVoiceChatDefault;
	HostRequestPtr->bUsePresence = true;
	QuickPlayRequest->bUseLobbies = bUseLobbiesDefault;

	NotifySessionInformationUpdated(EGameUserSessionInformationState::Matchmaking);
	FindSessionsInternal(JoiningOrHostingPlayer, CreateQuickPlaySearchSettings(HostRequest, QuickPlayRequest));
}

void UGameUserSessionSubsystem::JoinSession(APlayerController* JoiningPlayer, UGameUserSession_SearchResult* Request)
{
	if (Request == nullptr)
	{
		UE_LOG(LogGameUserSession, Error, TEXT("JoinSession passed a null request"));
		return;
	}

	ULocalPlayer* LocalPlayer = (JoiningPlayer != nullptr) ? JoiningPlayer->GetLocalPlayer() : nullptr;
	if (LocalPlayer == nullptr)
	{
		UE_LOG(LogGameUserSession, Error, TEXT("JoiningPlayer is invalid"));
		return;
	}

	// 在这里更新在线状态,因为客户端跳转后我们将不再拥有原始的 game mode 和 map name 键。如果加入/跳转失败,它会被重置回主菜单 
	FString SessionGameMode, SessionMapName;
	bool bEmpty;
	Request->GetStringSetting(SETTING_GAMEMODE, SessionGameMode, bEmpty);
	Request->GetStringSetting(SETTING_MAPNAME, SessionMapName, bEmpty);
	NotifySessionInformationUpdated(EGameUserSessionInformationState::InGame, SessionGameMode, SessionMapName);

	JoinSessionInternal(LocalPlayer, Request);
}

void UGameUserSessionSubsystem::FindSessions(APlayerController* SearchingPlayer, UGameUserSession_SearchSessionRequest* Request)
{
	if (Request == nullptr)
	{
		UE_LOG(LogGameUserSession, Error, TEXT("FindSessions passed a null request"));
		return;
	}

	FindSessionsInternal(SearchingPlayer, MakeShared<FGameOnlineSearchSettingsOSSv1>(Request));
}

void UGameUserSessionSubsystem::CleanUpSessions()
{
	bWantToDestroyPendingSession = true;

	if (bUseBeacons)
	{
		DestroyHostReservationBeacon();
	}

	NotifySessionInformationUpdated(EGameUserSessionInformationState::OutOfGame);
	CleanUpSessionsOSSv1();
}


TSharedRef<FGameOnlineSearchSettingsOSSv1> UGameUserSessionSubsystem::CreateQuickPlaySearchSettings(UGameUserSession_HostSessionRequest* HostRequest, UGameUserSession_SearchSessionRequest* SearchRequest)
{
	return CreateQuickPlaySearchSettingsOSSv1(HostRequest, SearchRequest);
}

void UGameUserSessionSubsystem::HandleQuickPlaySearchFinished(bool bSucceeded, const FText& ErrorMessage, TWeakObjectPtr<APlayerController> JoiningOrHostingPlayer, TStrongObjectPtr<UGameUserSession_HostSessionRequest> HostRequest)
{
	const int32 ResultCount = SearchSettings->SearchRequest->Results.Num();
	UE_LOG(LogGameUserSession, Log, TEXT("QuickPlay Search Finished %s (Results %d) (Error: %s)"), bSucceeded ? TEXT("Success") : TEXT("Failed"), ResultCount, *ErrorMessage.ToString());

	//@TODO: 我们必须检查错误消息是否为空,因为某些 OSS 层仅仅因为没有会话就报告失败。请在 OSS 2.0 中修复。
	if (bSucceeded || ErrorMessage.IsEmpty())
	{
		// 加入最优的搜索结果。
		if (ResultCount > 0)
		{
			//@TODO: 我们或许应该查看 ping?也许还有其他因素来找出最优结果。不确定它们是否已经预先排序。
			for (UGameUserSession_SearchResult* Result : SearchSettings->SearchRequest->Results)
			{
				JoinSession(JoiningOrHostingPlayer.Get(), Result);
				return;
			}
		}
		else
		{
			HostSession(JoiningOrHostingPlayer.Get(), HostRequest.Get());
		}
	}
	else
	{
		//@TODO: 这很糟糕,需要通知某人。
		NotifySessionInformationUpdated(EGameUserSessionInformationState::OutOfGame);
	}
}

void UGameUserSessionSubsystem::TravelLocalSessionFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ReasonString)
{
	// 此委托是全局的,但 PIE 可能有多个游戏实例,所以需要确保
	// 它针对的是此游戏实例子系统所关联的同一个世界(World)触发
	if (World != GetWorld())
	{
		return;
	}

	UE_LOG(LogGameUserSession, Warning, TEXT("TravelLocalSessionFailure(World: %s, FailureType: %s, ReasonString: %s)"),
	       *GetPathNameSafe(World),
	       ETravelFailure::ToString(FailureType),
	       *ReasonString);

	// TODO:  当我们也能广播成功时再广播此失败。目前我们在开始跳转之前就广播成功,因此成功之后再出现失败会令人困惑。
	//FOnlineResultInformation JoinSessionResult;
	//JoinSessionResult.bWasSuccessful = false;
	//JoinSessionResult.ErrorId = ReasonString; // TODO:  这个 ErrorId 是否合适?
	//JoinSessionResult.ErrorText = FText::FromString(ReasonString);
	//NotifyJoinSessionComplete(JoinSessionResult);
	NotifySessionInformationUpdated(EGameUserSessionInformationState::OutOfGame);
}

void UGameUserSessionSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogGameUserSession, Log, TEXT("OnCreateSessionComplete(SessionName: %s, bWasSuccessful: %d)"), *SessionName.ToString(), bWasSuccessful);

	// 如果存在分屏玩家,则将其加入
#if 0 //@TODO:
	if (bWasSuccessful && LocalPlayers.Num() > 1)
	{
		IOnlineSessionPtr Sessions = Online::GetSessionInterface(GetWorld());
		if (Sessions.IsValid() && LocalPlayers[1]->GetPreferredUniqueNetId().IsValid())
		{
			Sessions->RegisterLocalPlayer(*LocalPlayers[1]->GetPreferredUniqueNetId(), NAME_GameSession,
			                              FOnRegisterLocalPlayerCompleteDelegate::CreateUObject(this, &ThisClass::OnRegisterLocalPlayerComplete_CreateSession));
		}
	}
	else
#endif
	{
		// 我们要么失败了,要么只有一个本地用户
		FinishSessionCreation(bWasSuccessful);
	}
}

void UGameUserSessionSubsystem::FinishSessionCreation(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		//@TODO 将此处与加入回调的时机同步,如果计划有变,请同时修改两处及注释
		CreateSessionResult = FOnlineResultInformation();
		CreateSessionResult.bWasSuccessful = true;

		if (bUseBeacons)
		{
			CreateHostReservationBeacon();
		}

		NotifyCreateSessionComplete(CreateSessionResult);

		// 跳转到指定的比赛 URL
		GetWorld()->ServerTravel(PendingTravelURL);
	}
	else
	{
		if (CreateSessionResult.bWasSuccessful || CreateSessionResult.ErrorText.IsEmpty())
		{
			FString ReturnError = TEXT("GenericFailure"); // TODO: 没有好的方法从 OSSV1 中获取会话错误码
			FText ReturnReason = NSLOCTEXT("NetworkErrors", "CreateSessionFailed", "Failed to create session.");

			CreateSessionResult.bWasSuccessful = false;
			CreateSessionResult.ErrorId = ReturnError;
			CreateSessionResult.ErrorText = ReturnReason;
		}

		UE_LOG(LogGameUserSession, Error, TEXT("FinishSessionCreation(%s): %s"), *CreateSessionResult.ErrorId, *CreateSessionResult.ErrorText.ToString());

		NotifyCreateSessionComplete(CreateSessionResult);
		NotifySessionInformationUpdated(EGameUserSessionInformationState::OutOfGame);
	}
}

void UGameUserSessionSubsystem::HandlePostLoadMap(UWorld* World)
{
	// 忽略空的 World。
	if (!World)
	{
		return;
	}

	// 忽略任何不属于此游戏实例的 World,在编辑器中可能出现这种情况。
	if (World->GetGameInstance() != GetGameInstance())
	{
		return;
	}

	// 除非 World 类型是 game/pie,否则我们不关心更新会话。
	if (!(World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE))
	{
		return;
	}

	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);

	const IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
	check(SessionInterface.IsValid());

	const FName SessionName(NAME_GameSession);
	FNamedOnlineSession* CurrentSession = SessionInterface->GetNamedSession(SessionName);

	// 如果我们正在托管会话,则更新对外公布的地图名称。
	if (CurrentSession != nullptr && CurrentSession->bHosting)
	{
		// 这需要是完整的包路径,以匹配主机的 GetMapName 函数,World->GetMapName 目前是短名称 - 更新主机设置
		CurrentSession->SessionSettings.Set(SETTING_MAPNAME, UWorld::RemovePIEPrefix(World->GetOutermost()->GetName()), EOnlineDataAdvertisementType::ViaOnlineService);

		SessionInterface->UpdateSession(SessionName, CurrentSession->SessionSettings, true);

		if (bUseBeacons)
		{
			CreateHostReservationBeacon();
		}
	}
}

void UGameUserSessionSubsystem::BindOnlineDelegates()
{
	BindOnlineDelegatesOSSv1();
}

void UGameUserSessionSubsystem::CreateOnlineSessionInternal(ULocalPlayer* LocalPlayer, UGameUserSession_HostSessionRequest* Request)
{
	CreateSessionResult = FOnlineResultInformation();
	PendingTravelURL = Request->ConstructTravelURL();

	CreateOnlineSessionInternalOSSv1(LocalPlayer, Request);
}

void UGameUserSessionSubsystem::FindSessionsInternal(APlayerController* SearchingPlayer, const TSharedRef<FGameOnlineSearchSettingsOSSv1>& InSearchSettings)
{
	if (SearchSettings.IsValid())
	{
		//@TODO: 这对 API 使用者来说体验很差,我们应该让后续的搜索搭车执行,
		// 直接给它与当前进行中的搜索相同的结果
		// (或者将请求排队,在前一个搜索完成或失败后再处理它)
		UE_LOG(LogGameUserSession, Error, TEXT("A previous FindSessions call is still in progress, aborting"));
		SearchSettings->SearchRequest->NotifySearchFinished(false, LOCTEXT("Error_FindSessionAlreadyInProgress", "Session search already in progress"));
	}

	ULocalPlayer* LocalPlayer = (SearchingPlayer != nullptr) ? SearchingPlayer->GetLocalPlayer() : nullptr;
	if (LocalPlayer == nullptr)
	{
		UE_LOG(LogGameUserSession, Error, TEXT("SearchingPlayer is invalid"));
		InSearchSettings->SearchRequest->NotifySearchFinished(false, LOCTEXT("Error_FindSessionBadPlayer", "Session search was not provided a local player"));
		return;
	}

	SearchSettings = InSearchSettings;
	FindSessionsInternalOSSv1(LocalPlayer);
}

void UGameUserSessionSubsystem::JoinSessionInternal(ULocalPlayer* LocalPlayer, UGameUserSession_SearchResult* Request)
{
	JoinSessionInternalOSSv1(LocalPlayer, Request);
}

void UGameUserSessionSubsystem::InternalTravelToSession(const FName SessionName)
{
	//@TODO: 理想情况下我们应该使用触发跳转的玩家而不是第一个玩家(他们都会同时跳转,所以这可能无关紧要)
	APlayerController* const PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
	if (PlayerController == nullptr)
	{
		FText ReturnReason = NSLOCTEXT("NetworkErrors", "InvalidPlayerController", "Invalid Player Controller");
		UE_LOG(LogGameUserSession, Error, TEXT("InternalTravelToSession(Failed due to %s)"), *ReturnReason.ToString());
		return;
	}

	FString URL;
	// 跳转到会话
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);

	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	check(Sessions.IsValid());

	if (!Sessions->GetResolvedConnectString(SessionName, URL))
	{
		FText FailReason = NSLOCTEXT("NetworkErrors", "TravelSessionFailed", "Travel to Session failed.");
		UE_LOG(LogGameUserSession, Error, TEXT("InternalTravelToSession(%s)"), *FailReason.ToString());
		return;
	}

	// 允许在跳转之前修改 URL
	OnPreClientTravelEvent.Broadcast(URL);

	PlayerController->ClientTravel(URL, TRAVEL_Absolute);
}


void UGameUserSessionSubsystem::NotifyUserRequestedSession(const FPlatformUserId& PlatformUserId, UGameUserSession_SearchResult* RequestedSession, const FOnlineResultInformation& RequestedSessionResult)
{
	OnUserRequestedSessionEvent.Broadcast(PlatformUserId, RequestedSession, RequestedSessionResult);
	K2_OnUserRequestedSessionEvent.Broadcast(PlatformUserId, RequestedSession, RequestedSessionResult);
}

void UGameUserSessionSubsystem::NotifyJoinSessionComplete(const FOnlineResultInformation& Result)
{
	OnJoinSessionCompleteEvent.Broadcast(Result);
	K2_OnJoinSessionCompleteEvent.Broadcast(Result);
}

void UGameUserSessionSubsystem::NotifyCreateSessionComplete(const FOnlineResultInformation& Result)
{
	OnCreateSessionCompleteEvent.Broadcast(Result);
	K2_OnCreateSessionCompleteEvent.Broadcast(Result);
}


void UGameUserSessionSubsystem::NotifySessionInformationUpdated(EGameUserSessionInformationState SessionStatus, const FString& GameMode, const FString& MapName)
{
	OnSessionInformationChangedEvent.Broadcast(SessionStatus, GameMode, MapName);
	K2_OnSessionInformationChangedEvent.Broadcast(SessionStatus, GameMode, MapName);
}

void UGameUserSessionSubsystem::NotifyDestroySessionRequested(const FPlatformUserId& PlatformUserId, const FName& SessionName)
{
	OnDestroySessionRequestedEvent.Broadcast(PlatformUserId, SessionName);
	K2_OnDestroySessionRequestedEvent.Broadcast(PlatformUserId, SessionName);
}

void UGameUserSessionSubsystem::SetCreateSessionError(const FText& ErrorText)
{
	CreateSessionResult.bWasSuccessful = false;
	CreateSessionResult.ErrorId = TEXT("InternalFailure");

	// TODO 在发布构建中,根据你想给用户多少信息,可能想用通用错误文本替换
	CreateSessionResult.ErrorText = ErrorText;
}


void UGameUserSessionSubsystem::BindOnlineDelegatesOSSv1()
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);

	const IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
	check(SessionInterface.IsValid());

	SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete));
	SessionInterface->AddOnStartSessionCompleteDelegate_Handle(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete));
	SessionInterface->AddOnUpdateSessionCompleteDelegate_Handle(FOnUpdateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnUpdateSessionComplete));
	SessionInterface->AddOnEndSessionCompleteDelegate_Handle(FOnEndSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnEndSessionComplete));
	SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete));
	SessionInterface->AddOnDestroySessionRequestedDelegate_Handle(FOnDestroySessionRequestedDelegate::CreateUObject(this, &ThisClass::OnDestroySessionRequested));

	//	SessionInterface->AddOnMatchmakingCompleteDelegate_Handle(FOnMatchmakingCompleteDelegate::CreateUObject(this, &ThisClass::OnMatchmakingComplete));
	//	SessionInterface->AddOnCancelMatchmakingCompleteDelegate_Handle(FOnCancelMatchmakingCompleteDelegate::CreateUObject(this, &ThisClass::OnCancelMatchmakingComplete));

	SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete));
	// 	SessionInterface->AddOnCancelFindSessionsCompleteDelegate_Handle(FOnCancelFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnCancelFindSessionsComplete));
	// 	SessionInterface->AddOnPingSearchResultsCompleteDelegate_Handle(FOnPingSearchResultsCompleteDelegate::CreateUObject(this, &ThisClass::OnPingSearchResultsComplete));
	SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete));

	//	TWO_PARAM(OnSessionParticipantJoined, FName, const FUniqueNetId&);
	//	THREE_PARAM(OnSessionParticipantLeft, FName, const FUniqueNetId&, EOnSessionParticipantLeftReason);
	//	ONE_PARAM(OnQosDataRequested, FName);
	//	TWO_PARAM(OnSessionCustomDataChanged, FName, const FOnlineSessionSettings&);
	//	TWO_PARAM(OnSessionSettingsUpdated, FName, const FOnlineSessionSettings&);
	//	THREE_PARAM(OnSessionParticipantSettingsUpdated, FName, const FUniqueNetId&, const FOnlineSessionSettings&);
	//	FOUR_PARAM(OnSessionInviteReceived, const FUniqueNetId& /*UserId*/, const FUniqueNetId& /*FromId*/, const FString& /*AppId*/, const FOnlineSessionSearchResult& /*InviteResult*/);
	//	THREE_PARAM(OnRegisterPlayersComplete, FName, const TArray< FUniqueNetIdRef >&, bool);
	//	THREE_PARAM(OnUnregisterPlayersComplete, FName, const TArray< FUniqueNetIdRef >&, bool);

	SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &ThisClass::HandleSessionUserInviteAccepted));
	SessionInterface->AddOnSessionFailureDelegate_Handle(FOnSessionFailureDelegate::CreateUObject(this, &ThisClass::HandleSessionFailure));
}

void UGameUserSessionSubsystem::CreateOnlineSessionInternalOSSv1(ULocalPlayer* LocalPlayer, UGameUserSession_HostSessionRequest* Request)
{
	const FName SessionName(NAME_GameSession);
	const int32 MaxPlayers = Request->GetMaxPlayers();

	IOnlineSubsystem* const OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);

	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	check(Sessions);

	FUniqueNetIdPtr UserId;
	if (LocalPlayer)
	{
		UserId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
	}
	else if (bIsDedicatedServer)
	{
		UserId = OnlineSub->GetIdentityInterface()->GetUniquePlayerId(DEDICATED_SERVER_USER_INDEX);
	}

	//@TODO: 在某些平台上尝试创建 LAN 会话时可能会走到这里,这是否需要一个有效的用户 ID?
	if (ensure(UserId.IsValid()))
	{
		FGameUserSession_OnlineSessionSettings HostSettings(Request->OnlineMode == EGameUserSessionOnlineMode::LAN, Request->bUsePresence, MaxPlayers);
		HostSettings.bUseLobbiesIfAvailable = Request->bUseLobbies;
		HostSettings.bUseLobbiesVoiceChatIfAvailable = Request->bUseLobbiesVoiceChat;
		HostSettings.Set(SETTING_GAMEMODE, Request->ModeNameForAdvertisement, EOnlineDataAdvertisementType::ViaOnlineService);
		HostSettings.Set(SETTING_MAPNAME, Request->GetMapName(), EOnlineDataAdvertisementType::ViaOnlineService);
		//@TODO: HostSettings.Set(SETTING_MATCHING_HOPPER, FString("TeamDeathmatch"), EOnlineDataAdvertisementType::DontAdvertise);
		HostSettings.Set(SETTING_MATCHING_TIMEOUT, 120.0f, EOnlineDataAdvertisementType::ViaOnlineService);
		HostSettings.Set(SETTING_SESSION_TEMPLATE_NAME, FString(TEXT("GameSession")), EOnlineDataAdvertisementType::ViaOnlineService);
		HostSettings.Set(SETTING_ONLINESUBSYSTEM_VERSION, true, EOnlineDataAdvertisementType::ViaOnlineService);

		Sessions->CreateSession(*UserId, SessionName, HostSettings);
		NotifySessionInformationUpdated(EGameUserSessionInformationState::InGame, Request->ModeNameForAdvertisement, Request->GetMapName());
	}
	else
	{
		OnCreateSessionComplete(SessionName, false);
	}
}

void UGameUserSessionSubsystem::FindSessionsInternalOSSv1(ULocalPlayer* LocalPlayer)
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);
	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	check(Sessions);

	SearchSettings->QuerySettings.Set(SETTING_SESSION_TEMPLATE_NAME, FString("GameSession"), EOnlineComparisonOp::Equals);

	if (!Sessions->FindSessions(*LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), StaticCastSharedRef<FGameOnlineSearchSettingsOSSv1>(SearchSettings.ToSharedRef())))
	{
		// 某些会话搜索失败会在函数内部调用此委托,另一些则不会
		OnFindSessionsComplete(false);
	}
}

void UGameUserSessionSubsystem::JoinSessionInternalOSSv1(ULocalPlayer* LocalPlayer, UGameUserSession_SearchResult* Request)
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);
	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	check(Sessions);

	// 我们需要手动设置,表明我们希望这是我们的在线状态会话
	Request->Result.Session.SessionSettings.bUsesPresence = true;
	Request->Result.Session.SessionSettings.bUseLobbiesIfAvailable = bUseLobbiesDefault;

	Sessions->JoinSession(*LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), NAME_GameSession, Request->Result);
}

TSharedRef<FGameOnlineSearchSettingsOSSv1> UGameUserSessionSubsystem::CreateQuickPlaySearchSettingsOSSv1(UGameUserSession_HostSessionRequest* HostRequest, UGameUserSession_SearchSessionRequest* SearchRequest)
{
	TSharedRef<FGameOnlineSearchSettingsOSSv1> QuickPlaySearch = MakeShared<FGameOnlineSearchSettingsOSSv1>(SearchRequest);

	/** 默认情况下快速匹配不希望包含地图或游戏模式,游戏可以按需自行补充
	if (!HostRequest->ModeNameForAdvertisement.IsEmpty())
	{
		QuickPlaySearch->QuerySettings.Set(SETTING_GAMEMODE, HostRequest->ModeNameForAdvertisement, EOnlineComparisonOp::Equals);
	}

	if (!HostRequest->GetMapName().IsEmpty())
	{
		QuickPlaySearch->QuerySettings.Set(SETTING_MAPNAME, HostRequest->GetMapName(), EOnlineComparisonOp::Equals);
	} 
	*/

	// QuickPlaySearch->QuerySettings.Set(SEARCH_DEDICATED_ONLY, true, EOnlineComparisonOp::Equals);
	return QuickPlaySearch;
}

void UGameUserSessionSubsystem::CleanUpSessionsOSSv1()
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);
	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	check(Sessions);

	EOnlineSessionState::Type SessionState = Sessions->GetSessionState(NAME_GameSession);
	UE_LOG(LogGameUserSession, Log, TEXT("Session state is %s"), EOnlineSessionState::ToString(SessionState));

	if (EOnlineSessionState::InProgress == SessionState)
	{
		UE_LOG(LogGameUserSession, Log, TEXT("Ending session because of return to front end"));
		Sessions->EndSession(NAME_GameSession);
	}
	else if (EOnlineSessionState::Ending == SessionState)
	{
		UE_LOG(LogGameUserSession, Log, TEXT("Waiting for session to end on return to main menu"));
	}
	else if (EOnlineSessionState::Ended == SessionState || EOnlineSessionState::Pending == SessionState)
	{
		UE_LOG(LogGameUserSession, Log, TEXT("Destroying session on return to main menu"));
		Sessions->DestroySession(NAME_GameSession);
	}
	else if (EOnlineSessionState::Starting == SessionState || EOnlineSessionState::Creating == SessionState)
	{
		UE_LOG(LogGameUserSession, Log, TEXT("Waiting for session to start, and then we will end it to return to main menu"));
	}
}

void UGameUserSessionSubsystem::HandleSessionFailure(const FUniqueNetId& NetId, ESessionFailure::Type FailureType)
{
	UE_LOG(LogGameUserSession, Warning, TEXT("UGameUserSessionSubsystem::HandleSessionFailure(NetId: %s, FailureType: %s)"), *NetId.ToDebugString(), LexToString(FailureType));

	//@TODO: 可能需要做更多处理...
}

void UGameUserSessionSubsystem::HandleSessionUserInviteAccepted(const bool bWasSuccessful, const int32 LocalUserIndex, FUniqueNetIdPtr AcceptingUserId, const FOnlineSessionSearchResult& SearchResult)
{
	FPlatformUserId PlatformUserId = IPlatformInputDeviceMapper::Get().GetPlatformUserForUserIndex(LocalUserIndex);

	UGameUserSession_SearchResult* RequestedSession = nullptr;
	FOnlineResultInformation ResultInfo;
	if (bWasSuccessful)
	{
		RequestedSession = NewObject<UGameUserSession_SearchResult>(this);
		RequestedSession->Result = SearchResult;
	}
	else
	{
		// 没有可用的 FOnlineError 用于初始化
		ResultInfo.bWasSuccessful = false;
		ResultInfo.ErrorId = TEXT("failed"); // 这种方法不够健壮,但没有可用的扩展信息
		ResultInfo.ErrorText = LOCTEXT("Error_SessionUserInviteAcceptedFailed", "Failed to handle the join request");
	}
	NotifyUserRequestedSession(PlatformUserId, RequestedSession, ResultInfo);
}

void UGameUserSessionSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogGameUserSession, Log, TEXT("OnStartSessionComplete(SessionName: %s, bWasSuccessful: %d)"), *SessionName.ToString(), bWasSuccessful);

	if (bWantToDestroyPendingSession)
	{
		CleanUpSessions();
	}
}

void UGameUserSessionSubsystem::OnRegisterLocalPlayerComplete_CreateSession(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result)
{
	FinishSessionCreation(Result == EOnJoinSessionCompleteResult::Success);
}

void UGameUserSessionSubsystem::OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogGameUserSession, Log, TEXT("OnUpdateSessionComplete(SessionName: %s, bWasSuccessful: %s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));
}

void UGameUserSessionSubsystem::OnEndSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogGameUserSession, Log, TEXT("OnEndSessionComplete(SessionName: %s, bWasSuccessful: %s)"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));
	CleanUpSessions();
}

void UGameUserSessionSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogGameUserSession, Log, TEXT("OnDestroySessionComplete(SessionName: %s, bWasSuccessful: %s)"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));
	bWantToDestroyPendingSession = false;
}

void UGameUserSessionSubsystem::OnDestroySessionRequested(int32 LocalUserNum, FName SessionName)
{
	FPlatformUserId PlatformUserId = IPlatformInputDeviceMapper::Get().GetPlatformUserForUserIndex(LocalUserNum);

	NotifyDestroySessionRequested(PlatformUserId, SessionName);
}

void UGameUserSessionSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	UE_LOG(LogGameUserSession, Log, TEXT("OnFindSessionsComplete(bWasSuccessful: %s)"), bWasSuccessful ? TEXT("true") : TEXT("false"));

	if (!SearchSettings.IsValid())
	{
		// 对于失败的会话搜索,或者由其他系统发起的搜索,此函数可能会被调用两次
		return;
	}

	FGameOnlineSearchSettingsOSSv1& SearchSettingsV1 = *StaticCastSharedPtr<FGameOnlineSearchSettingsOSSv1>(SearchSettings);
	if (SearchSettingsV1.SearchState == EOnlineAsyncTaskState::InProgress)
	{
		UE_LOG(LogGameUserSession, Error, TEXT("OnFindSessionsComplete called when search is still in progress!"));
		return;
	}

	if (!ensure(SearchSettingsV1.SearchRequest))
	{
		UE_LOG(LogGameUserSession, Error, TEXT("OnFindSessionsComplete called with invalid search request object!"));
		return;
	}

	if (bWasSuccessful)
	{
		SearchSettingsV1.SearchRequest->Results.Reset(SearchSettingsV1.SearchResults.Num());

		for (const FOnlineSessionSearchResult& Result : SearchSettingsV1.SearchResults)
		{
			check(Result.IsValid());

			UGameUserSession_SearchResult* Entry = NewObject<UGameUserSession_SearchResult>(SearchSettingsV1.SearchRequest);
			Entry->Result = Result;
			SearchSettingsV1.SearchRequest->Results.Add(Entry);

			FString SessionId = TEXT("Unknown");
			if (Result.Session.SessionInfo.IsValid())
			{
				SessionId = Result.Session.SessionInfo->GetSessionId().ToString();
			}

			FString OwningUserId = TEXT("Unknown");
			if (Result.Session.OwningUserId.IsValid())
			{
				OwningUserId = Result.Session.OwningUserId->ToString();
			}

			UE_LOG(LogGameUserSession, Log, TEXT("\tFound session (SessionId: %s, UserId: %s, UserName: %s, NumOpenPrivConns: %d, NumOpenPubConns: %d, Ping: %d ms"),
			       *SessionId,
			       *OwningUserId,
			       *Result.Session.OwningUserName,
			       Result.Session.NumOpenPrivateConnections,
			       Result.Session.NumOpenPublicConnections,
			       Result.PingInMs
			);
		}
	}
	else
	{
		SearchSettingsV1.SearchRequest->Results.Empty();
	}

	if (0)
	{
		// 模拟会话 OSSV1
		for (int i = 0; i < 10; i++)
		{
			UGameUserSession_SearchResult* Entry = NewObject<UGameUserSession_SearchResult>(SearchSettings->SearchRequest);
			FOnlineSessionSearchResult FakeResult;
			FakeResult.Session.OwningUserName = TEXT("Fake User");
			FakeResult.Session.SessionSettings.NumPublicConnections = 10;
			FakeResult.Session.SessionSettings.bShouldAdvertise = true;
			FakeResult.Session.SessionSettings.bAllowJoinInProgress = true;
			FakeResult.PingInMs = 99;
			Entry->Result = FakeResult;
			SearchSettingsV1.SearchRequest->Results.Add(Entry);
		}
	}

	SearchSettingsV1.SearchRequest->NotifySearchFinished(bWasSuccessful, bWasSuccessful ? FText() : LOCTEXT("Error_FindSessionV1Failed", "Find session failed"));
	SearchSettings.Reset();
}

void UGameUserSessionSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	// 如果存在分屏玩家,则将其加入
	//@TODO:
	// 	if (Result == EOnJoinSessionCompleteResult::Success && LocalPlayers.Num() > 1)
	// 	{
	// 		IOnlineSessionPtr Sessions = Online::GetSessionInterface(GetWorld());
	// 		if (Sessions.IsValid() && LocalPlayers[1]->GetPreferredUniqueNetId().IsValid())
	// 		{
	// 			Sessions->RegisterLocalPlayer(*LocalPlayers[1]->GetPreferredUniqueNetId(), NAME_GameSession,
	// 				FOnRegisterLocalPlayerCompleteDelegate::CreateUObject(this, &UShooterGameInstance::OnRegisterJoiningLocalPlayerComplete));
	// 		}
	// 	}
	// 	else
	{
		FinishJoinSession(Result);
	}
}

void UGameUserSessionSubsystem::OnRegisterJoiningLocalPlayerComplete(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result)
{
	FinishJoinSession(Result);
}

void UGameUserSessionSubsystem::FinishJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		if (bUseBeacons)
		{
			// 在成功预留后,信标将调用 InternalTravelToSession 和通知。信标将在跳转过程中被销毁。
			ConnectToHostReservationBeacon();
		}
		else
		{
			//@TODO 将此处与创建回调的时机同步,如果计划有变,请同时修改两处及注释
			FOnlineResultInformation JoinSessionResult;
			JoinSessionResult.bWasSuccessful = true;
			NotifyJoinSessionComplete(JoinSessionResult);

			InternalTravelToSession(NAME_GameSession);
		}
	}
	else
	{
		FText ReturnReason;
		switch (Result)
		{
		case EOnJoinSessionCompleteResult::SessionIsFull:
			ReturnReason = NSLOCTEXT("NetworkErrors", "SessionIsFull", "Game is full.");
			break;
		case EOnJoinSessionCompleteResult::SessionDoesNotExist:
			ReturnReason = NSLOCTEXT("NetworkErrors", "SessionDoesNotExist", "Game no longer exists.");
			break;
		default:
			ReturnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Join failed.");
			break;
		}

		//@TODO: 错误处理
		UE_LOG(LogGameUserSession, Error, TEXT("FinishJoinSession(Failed with Result: %s)"), *ReturnReason.ToString());

		// 没有可用的 FOnlineError 用于初始化
		FOnlineResultInformation JoinSessionResult;
		JoinSessionResult.bWasSuccessful = false;
		JoinSessionResult.ErrorId = LexToString(Result); // 这种方法不够健壮,但没有可用的扩展信息
		JoinSessionResult.ErrorText = ReturnReason;
		NotifyJoinSessionComplete(JoinSessionResult);
		NotifySessionInformationUpdated(EGameUserSessionInformationState::OutOfGame);

		// 如果加入会话失败,我们将清理会话
		CleanUpSessions();
	}
}

void UGameUserSessionSubsystem::CreateHostReservationBeacon()
{
	check(!BeaconHostListener.IsValid());
	check(!ReservationBeaconHost.IsValid());

	UWorld* const World = GetWorld();
	BeaconHostListener = World->SpawnActor<AOnlineBeaconHost>(AOnlineBeaconHost::StaticClass());
	check(BeaconHostListener.IsValid());
	verify(BeaconHostListener->InitHost());

	ReservationBeaconHost = World->SpawnActor<APartyBeaconHost>(APartyBeaconHost::StaticClass());
	check(ReservationBeaconHost.IsValid());

	if (ReservationBeaconHostState)
	{
		ReservationBeaconHost->InitFromBeaconState(&*ReservationBeaconHostState);
	}
	else
	{
		// TODO: 目前我们使用默认的硬编码参数值,但这些参数是可配置的
		ReservationBeaconHost->InitHostBeacon(BeaconTeamCount, BeaconTeamSize, BeaconMaxReservations, NAME_GameSession);
		ReservationBeaconHostState = ReservationBeaconHost->GetState();
	}

	BeaconHostListener->RegisterHost(ReservationBeaconHost.Get());
	BeaconHostListener->PauseBeaconRequests(false);
}


void UGameUserSessionSubsystem::ConnectToHostReservationBeacon()
{
	UWorld* const World = GetWorld();
	check(World);
	ReservationBeaconClient = World->SpawnActor<APartyBeaconClient>(APartyBeaconClient::StaticClass());
	check(ReservationBeaconClient.IsValid());

	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(World);
	check(OnlineSub);
	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	check(Sessions);
	FNamedOnlineSession* Session = Sessions->GetNamedSession(NAME_GameSession);
	check(Session);
	FString SessionIdStr = Session->GetSessionIdStr();

	FString ConnectInfo;
	Sessions->GetResolvedConnectString(NAME_GameSession, ConnectInfo, NAME_BeaconPort);

	IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
	check(Identity);
	FUniqueNetIdWrapper DefaultNetId = Identity->GetUniquePlayerId(0);
	check(DefaultNetId.IsValid());

	FPlayerReservation PlayerReservation;
	PlayerReservation.UniqueId = *DefaultNetId;
	PlayerReservation.Platform = OnlineSub->GetLocalPlatformName();

	ReservationBeaconClient->OnHostConnectionFailure().BindWeakLambda(this, [this]()
	{
		// 我们只希望在连接处于活动状态时对失败调用做出反应,而不是在连接关闭时
		if (ReservationBeaconClient->GetNetDriver())
		{
			FOnlineResultInformation JoinSessionResult;
			JoinSessionResult.bWasSuccessful = false;
			JoinSessionResult.ErrorId = TEXT("UnknownError");

			NotifyJoinSessionComplete(JoinSessionResult);
			NotifySessionInformationUpdated(EGameUserSessionInformationState::OutOfGame);

			CleanUpSessions();
		}
	});

	ReservationBeaconClient->OnReservationRequestComplete().BindWeakLambda(this, [this](EPartyReservationResult::Type ReservationResponse)
	{
		if (ReservationResponse == EPartyReservationResult::ReservationAccepted || ReservationResponse == EPartyReservationResult::ReservationDuplicate)
		{
			FOnlineResultInformation JoinSessionResult;
			JoinSessionResult.bWasSuccessful = true;
			NotifyJoinSessionComplete(JoinSessionResult);

			InternalTravelToSession(NAME_GameSession);
		}
		else
		{
			FOnlineResultInformation JoinSessionResult;
			JoinSessionResult.bWasSuccessful = false;
			JoinSessionResult.ErrorId = TEXT("UnknownError");

			NotifyJoinSessionComplete(JoinSessionResult);
			NotifySessionInformationUpdated(EGameUserSessionInformationState::OutOfGame);

			CleanUpSessions();
		}
	});

	ReservationBeaconClient->RequestReservation(ConnectInfo, SessionIdStr, *DefaultNetId, {PlayerReservation});
}

void UGameUserSessionSubsystem::DestroyHostReservationBeacon()
{
	if (BeaconHostListener.IsValid() && ReservationBeaconHost.IsValid())
	{
		BeaconHostListener->UnregisterHost(ReservationBeaconHost->GetBeaconType());
	}
	if (BeaconHostListener.IsValid())
	{
		BeaconHostListener->Destroy();
		BeaconHostListener = nullptr;
	}
	if (ReservationBeaconHost.IsValid())
	{
		ReservationBeaconHost->Destroy();
		ReservationBeaconHost = nullptr;
	}
}

#undef LOCTEXT_NAMESPACE
