// Copyright © 2026 张鸿源. All Rights Reserved.

#include "System/GameUserSubsystem.h"
#include "InputKeyEventArgs.h"
#include "LogGameUser.h"
#include "OnlineSubsystemNames.h"
#include "OnlineSubsystemUtils.h"
#include "TimerManager.h"
#include "Data/GameUserInfo.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameUserSubsystem)

void UGameUserSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 创建我们的 OSS 包装器
	CreateOnlineContexts();

	BindOnlineDelegates();

	IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();
	DeviceMapper.GetOnInputDeviceConnectionChange().AddUObject(this, &ThisClass::HandleInputDeviceConnectionChanged);

	// 与引擎默认值一致
	SetMaxLocalPlayers(4);

	ResetUserState();

	UGameInstance* GameInstance = GetGameInstance();
	bIsDedicatedServer = GameInstance->IsDedicatedServerInstance();
}

void UGameUserSubsystem::Deinitialize()
{
	DestroyOnlineContexts();

	IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();
	DeviceMapper.GetOnInputDeviceConnectionChange().RemoveAll(this);

	LocalUserInfos.Reset();
	ActiveLoginRequests.Reset();

	Super::Deinitialize();
}

bool UGameUserSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	TArray<UClass*> ChildClasses;
	GetDerivedClasses(GetClass(), ChildClasses, false);

	// 仅当不存在游戏特定的子类时才创建实例
	return ChildClasses.Num() == 0;
}

void UGameUserSubsystem::SendSystemMessage(FGameplayTag MessageType, FText TitleText, FText BodyText)
{
	OnHandleSystemMessage.Broadcast(MessageType, TitleText, BodyText);
}

void UGameUserSubsystem::SetMaxLocalPlayers(int32 InMaxLocalPlayers)
{
	if (ensure(InMaxLocalPlayers >= 1))
	{
		// 本地玩家数量可以超过 MAX_LOCAL_PLAYERS,其余玩家将被视为游客
		MaxNumberOfLocalPlayers = InMaxLocalPlayers;

		UGameInstance* GameInstance = GetGameInstance();
		UGameViewportClient* ViewportClient = GameInstance ? GameInstance->GetGameViewportClient() : nullptr;

		if (ViewportClient)
		{
			ViewportClient->MaxSplitscreenPlayers = MaxNumberOfLocalPlayers;
		}
	}
}

int32 UGameUserSubsystem::GetMaxLocalPlayers() const
{
	return MaxNumberOfLocalPlayers;
}

int32 UGameUserSubsystem::GetNumLocalPlayers() const
{
	UGameInstance* GameInstance = GetGameInstance();
	if (ensure(GameInstance))
	{
		return GameInstance->GetNumLocalPlayers();
	}
	return 1;
}

EGameUserInitializationState UGameUserSubsystem::GetLocalPlayerInitializationState(int32 LocalPlayerIndex) const
{
	const UGameUserInfo* UserInfo = GetUserInfoForLocalPlayerIndex(LocalPlayerIndex);
	if (UserInfo)
	{
		return UserInfo->InitializationState;
	}

	if (LocalPlayerIndex < 0 || LocalPlayerIndex >= GetMaxLocalPlayers())
	{
		return EGameUserInitializationState::Invalid;
	}

	return EGameUserInitializationState::Unknown;
}

bool UGameUserSubsystem::TryToInitializeForLocalPlay(int32 LocalPlayerIndex, FInputDeviceId PrimaryInputDevice, bool bCanUseGuestLogin)
{
	if (!PrimaryInputDevice.IsValid())
	{
		// 设置为默认设备
		PrimaryInputDevice = IPlatformInputDeviceMapper::Get().GetDefaultInputDevice();
	}

	FGameUserInitializeParams Params;
	Params.LocalPlayerIndex = LocalPlayerIndex;
	Params.PrimaryInputDevice = PrimaryInputDevice;
	Params.bCanUseGuestLogin = bCanUseGuestLogin;
	Params.bCanCreateNewLocalPlayer = true;
	Params.RequestedPrivilege = EGameUserPrivilege::CanPlay;

	return TryToInitializeUser(Params);
}

bool UGameUserSubsystem::TryToLoginForOnlinePlay(int32 LocalPlayerIndex)
{
	FGameUserInitializeParams Params;
	Params.LocalPlayerIndex = LocalPlayerIndex;
	Params.bCanCreateNewLocalPlayer = false;
	Params.RequestedPrivilege = EGameUserPrivilege::CanPlayOnline;

	return TryToInitializeUser(Params);
}

bool UGameUserSubsystem::TryToInitializeUser(FGameUserInitializeParams Params)
{
	if (Params.LocalPlayerIndex < 0 || (!Params.bCanCreateNewLocalPlayer && Params.LocalPlayerIndex >= GetNumLocalPlayers()))
	{
		if (!bIsDedicatedServer)
		{
			UE_LOG(LogGameUser, Error, TEXT("TryToInitializeUser %d failed with current %d and max %d, invalid index"),
			       Params.LocalPlayerIndex, GetNumLocalPlayers(), GetMaxLocalPlayers());
			return false;
		}
	}

	if (Params.LocalPlayerIndex > GetNumLocalPlayers() || Params.LocalPlayerIndex >= GetMaxLocalPlayers())
	{
		UE_LOG(LogGameUser, Error, TEXT("TryToInitializeUser %d failed with current %d and max %d, can only create in order up to max players"),
		       Params.LocalPlayerIndex, GetNumLocalPlayers(), GetMaxLocalPlayers());
		return false;
	}

	// 如有需要,填充平台用户和输入设备
	if (Params.ControllerId != INDEX_NONE && (!Params.PrimaryInputDevice.IsValid() || !Params.PlatformUser.IsValid()))
	{
		IPlatformInputDeviceMapper::Get().RemapControllerIdToPlatformUserAndDevice(Params.ControllerId, Params.PlatformUser, Params.PrimaryInputDevice);
	}

	if (Params.PrimaryInputDevice.IsValid() && !Params.PlatformUser.IsValid())
	{
		Params.PlatformUser = GetPlatformUserIdForInputDevice(Params.PrimaryInputDevice);
	}
	else if (Params.PlatformUser.IsValid() && !Params.PrimaryInputDevice.IsValid())
	{
		Params.PrimaryInputDevice = GetPrimaryInputDeviceForPlatformUser(Params.PlatformUser);
	}

	UGameUserInfo* LocalUserInfo = ModifyInfo(GetUserInfoForLocalPlayerIndex(Params.LocalPlayerIndex));
	UGameUserInfo* LocalUserInfoForController = ModifyInfo(GetUserInfoForInputDevice(Params.PrimaryInputDevice));

	if (LocalUserInfoForController && LocalUserInfo && LocalUserInfoForController != LocalUserInfo)
	{
		UE_LOG(LogGameUser, Error, TEXT("TryToInitializeUser %d failed because controller %d is already assigned to player %d"),
		       Params.LocalPlayerIndex, Params.PrimaryInputDevice.GetId(), LocalUserInfoForController->LocalPlayerIndex);
		return false;
	}

	if (Params.LocalPlayerIndex == 0 && Params.bCanUseGuestLogin)
	{
		UE_LOG(LogGameUser, Error, TEXT("TryToInitializeUser failed because player 0 cannot be a guest"));
		return false;
	}

	if (!LocalUserInfo)
	{
		LocalUserInfo = CreateLocalUserInfo(Params.LocalPlayerIndex);
	}
	else
	{
		// 从现有用户信息中复制
		if (!Params.PrimaryInputDevice.IsValid())
		{
			Params.PrimaryInputDevice = LocalUserInfo->PrimaryInputDevice;
		}

		if (!Params.PlatformUser.IsValid())
		{
			Params.PlatformUser = LocalUserInfo->PlatformUser;
		}
	}

	if (LocalUserInfo->InitializationState != EGameUserInitializationState::Unknown && LocalUserInfo->InitializationState != EGameUserInitializationState::FailedtoLogin)
	{
		// 登录期间不允许更改参数
		if (LocalUserInfo->PrimaryInputDevice != Params.PrimaryInputDevice || LocalUserInfo->PlatformUser != Params.PlatformUser || LocalUserInfo->bCanBeGuest != Params.bCanUseGuestLogin)
		{
			UE_LOG(LogGameUser, Error, TEXT("TryToInitializeUser failed because player %d has already started the login process with diffrent settings!"), Params.LocalPlayerIndex);
			return false;
		}
	}

	// 现在设置期望的索引,以便在创建玩家时知道要使用哪个手柄
	LocalUserInfo->PrimaryInputDevice = Params.PrimaryInputDevice;
	LocalUserInfo->PlatformUser = Params.PlatformUser;
	LocalUserInfo->bCanBeGuest = Params.bCanUseGuestLogin;
	RefreshLocalUserInfo(LocalUserInfo);

	// 要么进行初始登录,要么进行网络登录
	if (LocalUserInfo->GetPrivilegeAvailability(EGameUserPrivilege::CanPlay) == EGameUserAvailability::NowAvailable && Params.RequestedPrivilege == EGameUserPrivilege::CanPlayOnline)
	{
		LocalUserInfo->InitializationState = EGameUserInitializationState::DoingNetworkLogin;
	}
	else
	{
		LocalUserInfo->InitializationState = EGameUserInitializationState::DoingInitialLogin;
	}

	LoginLocalUser(LocalUserInfo, Params.RequestedPrivilege, Params.OnlineContext, FOnLocalUserLoginCompleteDelegate::CreateUObject(this, &ThisClass::HandleLoginForUserInitialize, Params));

	return true;
}

void UGameUserSubsystem::ListenForLoginKeyInput(TArray<FKey> AnyUserKeys, TArray<FKey> NewUserKeys, FGameUserInitializeParams Params)
{
	UGameViewportClient* ViewportClient = GetGameInstance()->GetGameViewportClient();
	if (ensure(ViewportClient))
	{
		const bool bIsMapped = LoginKeysForAnyUser.Num() > 0 || LoginKeysForNewUser.Num() > 0;
		const bool bShouldBeMapped = AnyUserKeys.Num() > 0 || NewUserKeys.Num() > 0;

		if (bIsMapped && !bShouldBeMapped)
		{
			// 恢复为被包装的处理器
			ViewportClient->OnOverrideInputKey() = WrappedInputKeyHandler;
			WrappedInputKeyHandler.Unbind();
		}
		else if (!bIsMapped && bShouldBeMapped)
		{
			// 设置一个包装处理器
			WrappedInputKeyHandler = ViewportClient->OnOverrideInputKey();
			ViewportClient->OnOverrideInputKey().BindUObject(this, &UGameUserSubsystem::OverrideInputKeyForLogin);
		}

		LoginKeysForAnyUser = AnyUserKeys;
		LoginKeysForNewUser = NewUserKeys;

		if (bShouldBeMapped)
		{
			ParamsForLoginKey = Params;
		}
		else
		{
			ParamsForLoginKey = FGameUserInitializeParams();
		}
	}
}


bool UGameUserSubsystem::CancelUserInitialization(int32 LocalPlayerIndex)
{
	UGameUserInfo* LocalUserInfo = ModifyInfo(GetUserInfoForLocalPlayerIndex(LocalPlayerIndex));
	if (!LocalUserInfo)
	{
		return false;
	}

	if (!LocalUserInfo->IsDoingLogin())
	{
		return false;
	}

	// 从登录队列中移除
	TArray<TSharedRef<FUserLoginRequest>> RequestsCopy = ActiveLoginRequests;
	for (TSharedRef<FUserLoginRequest>& Request : RequestsCopy)
	{
		if (Request->UserInfo.IsValid() && Request->UserInfo->LocalPlayerIndex == LocalPlayerIndex)
		{
			ActiveLoginRequests.Remove(Request);
		}
	}

	// 以最合理的猜测设置状态
	if (LocalUserInfo->InitializationState == EGameUserInitializationState::DoingNetworkLogin)
	{
		LocalUserInfo->InitializationState = EGameUserInitializationState::LoggedInLocalOnly;
	}
	else
	{
		LocalUserInfo->InitializationState = EGameUserInitializationState::FailedtoLogin;
	}

	return true;
}

bool UGameUserSubsystem::TryToLogOutUser(int32 LocalPlayerIndex, bool bDestroyPlayer)
{
	UGameInstance* GameInstance = GetGameInstance();

	if (!ensure(GameInstance))
	{
		return false;
	}

	if (LocalPlayerIndex == 0 && bDestroyPlayer)
	{
		UE_LOG(LogGameUser, Error, TEXT("TryToLogOutUser cannot destroy player 0"));
		return false;
	}

	CancelUserInitialization(LocalPlayerIndex);

	UGameUserInfo* LocalUserInfo = ModifyInfo(GetUserInfoForLocalPlayerIndex(LocalPlayerIndex));
	if (!LocalUserInfo)
	{
		UE_LOG(LogGameUser, Warning, TEXT("TryToLogOutUser failed to log out user %i because they are not logged in"), LocalPlayerIndex);
		return false;
	}

	FPlatformUserId UserId = LocalUserInfo->PlatformUser;
	if (IsRealPlatformUser(UserId))
	{
		// 目前这里不执行平台登出,以防用户紧接着要重新登录
		UE_LOG(LogGameUser, Log, TEXT("TryToLogOutUser succeeded for real platform user %d"), UserId.GetInternalId());

		LogOutLocalUser(UserId);
	}
	else if (ensure(LocalUserInfo->bIsGuest))
	{
		// 对于游客用户,直接删除即可
		UE_LOG(LogGameUser, Log, TEXT("TryToLogOutUser succeeded for guest player index %d"), LocalPlayerIndex);

		LocalUserInfos.Remove(LocalPlayerIndex);
	}

	if (bDestroyPlayer)
	{
		ULocalPlayer* ExistingPlayer = GameInstance->FindLocalPlayerFromPlatformUserId(UserId);

		if (ExistingPlayer)
		{
			GameInstance->RemoveLocalPlayer(ExistingPlayer);
		}
	}

	return true;
}

void UGameUserSubsystem::ResetUserState()
{
	// 手动清理现有的信息对象
	for (TPair<int32, UGameUserInfo*> Pair : LocalUserInfos)
	{
		if (Pair.Value)
		{
			Pair.Value->MarkAsGarbage();
		}
	}

	LocalUserInfos.Reset();

	// 取消进行中的登录
	ActiveLoginRequests.Reset();

	// 为 ID 0 创建玩家信息
	UGameUserInfo* FirstUser = CreateLocalUserInfo(0);

	FirstUser->PlatformUser = IPlatformInputDeviceMapper::Get().GetPrimaryPlatformUser();
	FirstUser->PrimaryInputDevice = IPlatformInputDeviceMapper::Get().GetPrimaryInputDeviceForUser(FirstUser->PlatformUser);

	// TODO: 是否安排下一帧刷新玩家 0?
	RefreshLocalUserInfo(FirstUser);
}

IOnlineSubsystem* UGameUserSubsystem::GetOnlineSubsystem(EGameUserOnlineContext Context) const
{
	const FOnlineContextCache* System = GetContextCache(Context);

	if (System)
	{
		return System->OnlineSubsystem;
	}

	return nullptr;
}

IOnlineIdentity* UGameUserSubsystem::GetOnlineIdentity(EGameUserOnlineContext Context) const
{
	const FOnlineContextCache* System = GetContextCache(Context);
	if (System)
	{
		return System->IdentityInterface.Get();
	}

	return nullptr;
}

FName UGameUserSubsystem::GetOnlineSubsystemName(EGameUserOnlineContext Context) const
{
	IOnlineSubsystem* SubSystem = GetOnlineSubsystem(Context);
	if (SubSystem)
	{
		return SubSystem->GetSubsystemName();
	}

	return NAME_None;
}

EOnlineServerConnectionStatus::Type UGameUserSubsystem::GetConnectionStatus(EGameUserOnlineContext Context) const
{
	const FOnlineContextCache* System = GetContextCache(Context);
	if (System)
	{
		return System->CurrentConnectionStatus;
	}

	return EOnlineServerConnectionStatus::ServiceUnavailable;
}

bool UGameUserSubsystem::HasOnlineConnection(EGameUserOnlineContext Context) const
{
	EOnlineServerConnectionStatus::Type ConnectionType = GetConnectionStatus(Context);

	if (ConnectionType == EOnlineServerConnectionStatus::Normal || ConnectionType == EOnlineServerConnectionStatus::Connected)
	{
		return true;
	}

	return false;
}


ELoginStatusType UGameUserSubsystem::GetLocalUserLoginStatus(FPlatformUserId PlatformUser, EGameUserOnlineContext Context) const
{
	if (!IsRealPlatformUser(PlatformUser))
	{
		return ELoginStatusType::NotLoggedIn;
	}

	const FOnlineContextCache* System = GetContextCache(Context);
	if (System)
	{
		return System->IdentityInterface->GetLoginStatus(GetPlatformUserIndexForId(PlatformUser));
	}
	return ELoginStatusType::NotLoggedIn;
}

FUniqueNetIdRepl UGameUserSubsystem::GetLocalUserNetId(FPlatformUserId PlatformUser, EGameUserOnlineContext Context) const
{
	if (!IsRealPlatformUser(PlatformUser))
	{
		return FUniqueNetIdRepl();
	}

	const FOnlineContextCache* System = GetContextCache(Context);
	if (System)
	{
		return FUniqueNetIdRepl(System->IdentityInterface->GetUniquePlayerId(GetPlatformUserIndexForId(PlatformUser)));
	}

	return FUniqueNetIdRepl();
}

FString UGameUserSubsystem::GetLocalUserNickname(FPlatformUserId PlatformUser, EGameUserOnlineContext Context) const
{
	IOnlineIdentity* Identity = GetOnlineIdentity(Context);
	if (ensure(Identity))
	{
		return Identity->GetPlayerNickname(GetPlatformUserIndexForId(PlatformUser));
	}

	return FString();
}

UGameUserInfo* UGameUserSubsystem::CreateLocalUserInfo(int32 LocalPlayerIndex)
{
	UGameUserInfo* NewUser = nullptr;
	if (ensure(!LocalUserInfos.Contains(LocalPlayerIndex)))
	{
		NewUser = NewObject<UGameUserInfo>(this);
		NewUser->LocalPlayerIndex = LocalPlayerIndex;
		NewUser->InitializationState = EGameUserInitializationState::Unknown;

		// 始终创建游戏(Game)和默认(Default)缓存
		NewUser->CachedDataMap.Add(EGameUserOnlineContext::Game, UGameUserInfo::FCachedData());
		NewUser->CachedDataMap.Add(EGameUserOnlineContext::Default, UGameUserInfo::FCachedData());

		// 如有需要,添加平台(Platform)上下文
		if (HasSeparatePlatformContext())
		{
			NewUser->CachedDataMap.Add(EGameUserOnlineContext::Platform, UGameUserInfo::FCachedData());
		}

		LocalUserInfos.Add(LocalPlayerIndex, NewUser);
	}
	return NewUser;
}

void UGameUserSubsystem::CreateOnlineContexts()
{
	// 首先初始化默认上下文
	DefaultContextInternal = new FOnlineContextCache();
	DefaultContextInternal->OnlineSubsystem = Online::GetSubsystem(GetWorld());
	check(DefaultContextInternal->OnlineSubsystem);
	DefaultContextInternal->IdentityInterface = DefaultContextInternal->OnlineSubsystem->GetIdentityInterface();
	check(DefaultContextInternal->IdentityInterface.IsValid());

	IOnlineSubsystem* PlatformSub = IOnlineSubsystem::GetByPlatform();

	if (PlatformSub && DefaultContextInternal->OnlineSubsystem != PlatformSub)
	{
		// 如果存在,设置可选的平台服务
		PlatformContextInternal = new FOnlineContextCache();
		PlatformContextInternal->OnlineSubsystem = PlatformSub;
		PlatformContextInternal->IdentityInterface = PlatformSub->GetIdentityInterface();
		check(PlatformContextInternal->IdentityInterface.IsValid());
	}

	// 如有需要,之后可以显式设置外部服务
}

void UGameUserSubsystem::DestroyOnlineContexts()
{
	// 所有缓存的共享指针都必须在这里清除
	if (ServiceContextInternal && ServiceContextInternal != DefaultContextInternal)
	{
		delete ServiceContextInternal;
	}
	if (PlatformContextInternal && PlatformContextInternal != DefaultContextInternal)
	{
		delete PlatformContextInternal;
	}
	if (DefaultContextInternal)
	{
		delete DefaultContextInternal;
	}

	ServiceContextInternal = PlatformContextInternal = DefaultContextInternal = nullptr;
}

void UGameUserSubsystem::BindOnlineDelegates()
{
	return BindOnlineDelegatesOSSv1();
}

void UGameUserSubsystem::LogOutLocalUser(FPlatformUserId PlatformUser)
{
	UGameUserInfo* UserInfo = ModifyInfo(GetUserInfoForPlatformUser(PlatformUser));

	// 如果用户从未完全登录过,或正处于登录过程中,则无需执行任何操作
	if (UserInfo && (UserInfo->InitializationState == EGameUserInitializationState::LoggedInLocalOnly || UserInfo->InitializationState == EGameUserInitializationState::LoggedInOnline))
	{
		EGameUserAvailability OldAvailablity = UserInfo->GetPrivilegeAvailability(EGameUserPrivilege::CanPlay);

		UserInfo->InitializationState = EGameUserInitializationState::FailedtoLogin;

		// 这将广播游戏委托
		HandleChangedAvailability(UserInfo, EGameUserPrivilege::CanPlay, OldAvailablity);
	}
}

bool UGameUserSubsystem::TransferPlatformAuth(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	// V1 路径不支持
	return false;
}

bool UGameUserSubsystem::AutoLogin(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	UE_LOG(LogGameUser, Log, TEXT("Player AutoLogin requested - UserIdx:%d, Privilege:%d, Context:%d"),
	       PlatformUser.GetInternalId(),
	       (int32)Request->DesiredPrivilege,
	       (int32)Request->DesiredContext);

	return AutoLoginOSSv1(System, Request, PlatformUser);
}

bool UGameUserSubsystem::ShowLoginUI(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	UE_LOG(LogGameUser, Log, TEXT("Player LoginUI requested - UserIdx:%d, Privilege:%d, Context:%d"),
	       PlatformUser.GetInternalId(),
	       (int32)Request->DesiredPrivilege,
	       (int32)Request->DesiredContext);

	return ShowLoginUIOSSv1(System, Request, PlatformUser);
}

bool UGameUserSubsystem::QueryUserPrivilege(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	return QueryUserPrivilegeOSSv1(System, Request, PlatformUser);
}

void UGameUserSubsystem::BindOnlineDelegatesOSSv1()
{
	EGameUserOnlineContext ServiceType = ResolveOnlineContext(EGameUserOnlineContext::ServiceOrDefault);
	EGameUserOnlineContext PlatformType = ResolveOnlineContext(EGameUserOnlineContext::PlatformOrDefault);
	FOnlineContextCache* ServiceContext = GetContextCache(ServiceType);
	FOnlineContextCache* PlatformContext = GetContextCache(PlatformType);
	check(ServiceContext && ServiceContext->OnlineSubsystem && PlatformContext && PlatformContext->OnlineSubsystem);
	// 连接委托需要同时监听两个系统

	ServiceContext->OnlineSubsystem->AddOnConnectionStatusChangedDelegate_Handle(FOnConnectionStatusChangedDelegate::CreateUObject(this, &ThisClass::HandleNetworkConnectionStatusChanged, ServiceType));
	ServiceContext->CurrentConnectionStatus = EOnlineServerConnectionStatus::Normal;

	for (int32 PlayerIdx = 0; PlayerIdx < MAX_LOCAL_PLAYERS; PlayerIdx++)
	{
		ServiceContext->IdentityInterface->AddOnLoginStatusChangedDelegate_Handle(PlayerIdx, FOnLoginStatusChangedDelegate::CreateUObject(this, &ThisClass::HandleIdentityLoginStatusChanged, ServiceType));
		ServiceContext->IdentityInterface->AddOnLoginCompleteDelegate_Handle(PlayerIdx, FOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::HandleUserLoginCompleted, ServiceType));
	}

	if (ServiceType != PlatformType)
	{
		PlatformContext->OnlineSubsystem->AddOnConnectionStatusChangedDelegate_Handle(FOnConnectionStatusChangedDelegate::CreateUObject(this, &ThisClass::HandleNetworkConnectionStatusChanged, PlatformType));
		PlatformContext->CurrentConnectionStatus = EOnlineServerConnectionStatus::Normal;

		for (int32 PlayerIdx = 0; PlayerIdx < MAX_LOCAL_PLAYERS; PlayerIdx++)
		{
			PlatformContext->IdentityInterface->AddOnLoginStatusChangedDelegate_Handle(PlayerIdx, FOnLoginStatusChangedDelegate::CreateUObject(this, &ThisClass::HandleIdentityLoginStatusChanged, PlatformType));
			PlatformContext->IdentityInterface->AddOnLoginCompleteDelegate_Handle(PlayerIdx, FOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::HandleUserLoginCompleted, PlatformType));
		}
	}

	// 硬件变化委托只监听平台系统
	PlatformContext->IdentityInterface->AddOnControllerPairingChangedDelegate_Handle(FOnControllerPairingChangedDelegate::CreateUObject(this, &ThisClass::HandleControllerPairingChanged));
}

bool UGameUserSubsystem::AutoLoginOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	return System->IdentityInterface->AutoLogin(GetPlatformUserIndexForId(PlatformUser));
}

bool UGameUserSubsystem::ShowLoginUIOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	IOnlineExternalUIPtr ExternalUI = System->OnlineSubsystem->GetExternalUIInterface();
	if (ExternalUI.IsValid())
	{
		// TODO 不清楚应该设置哪些标志位
		return ExternalUI->ShowLoginUI(GetPlatformUserIndexForId(PlatformUser), false, false, FOnLoginUIClosedDelegate::CreateUObject(this, &ThisClass::HandleOnLoginUIClosed, Request->CurrentContext));
	}
	return false;
}

bool UGameUserSubsystem::QueryUserPrivilegeOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser)
{
	// 在状态未知或失败时开始查询
	EUserPrivileges::Type OSSPrivilege = ConvertOSSPrivilege(Request->DesiredPrivilege);

	FUniqueNetIdRepl CurrentId = GetLocalUserNetId(PlatformUser, Request->CurrentContext);
	check(CurrentId.IsValid());
	IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate Delegate = IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate::CreateUObject(this, &UGameUserSubsystem::HandleCheckPrivilegesComplete, Request->DesiredPrivilege, Request->UserInfo,
	                                                                                                                                    Request->CurrentContext);
	System->IdentityInterface->GetUserPrivilege(*CurrentId, OSSPrivilege, Delegate);

	// 这可能会立即成功并重新进入此函数,因此我们必须返回
	return true;
}

bool UGameUserSubsystem::OverrideInputKeyForLogin(FInputKeyEventArgs& EventArgs)
{
	int32 NextLocalPlayerIndex = INDEX_NONE;

	const UGameUserInfo* MappedUser = GetUserInfoForInputDevice(EventArgs.InputDevice);
	if (EventArgs.Event == IE_Pressed)
	{
		if (MappedUser == nullptr || !MappedUser->IsLoggedIn())
		{
			if (MappedUser)
			{
				NextLocalPlayerIndex = MappedUser->LocalPlayerIndex;
			}
			else
			{
				// 查找下一个玩家
				for (int32 i = 0; i < MaxNumberOfLocalPlayers; i++)
				{
					if (GetLocalPlayerInitializationState(i) == EGameUserInitializationState::Unknown)
					{
						NextLocalPlayerIndex = i;
						break;
					}
				}
			}

			if (NextLocalPlayerIndex != INDEX_NONE)
			{
				if (LoginKeysForAnyUser.Contains(EventArgs.Key))
				{
					// 如果正在进行登录,直接返回 true 以忽略平台特定的输入
					if (MappedUser && MappedUser->IsDoingLogin())
					{
						return true;
					}

					// 按下开始键(开始界面)
					FGameUserInitializeParams NewParams = ParamsForLoginKey;
					NewParams.LocalPlayerIndex = NextLocalPlayerIndex;
					NewParams.PrimaryInputDevice = EventArgs.InputDevice;

					return TryToInitializeUser(NewParams);
				}

				// 检查该手柄 ID 是否已被映射
				MappedUser = GetUserInfoForInputDevice(EventArgs.InputDevice);

				if (!MappedUser || MappedUser->LocalPlayerIndex == INDEX_NONE)
				{
					if (LoginKeysForNewUser.Contains(EventArgs.Key))
					{
						// 如果正在进行登录,直接返回 true 以忽略平台特定的输入
						if (MappedUser && MappedUser->IsDoingLogin())
						{
							return true;
						}

						// 本地多人游戏
						FGameUserInitializeParams NewParams = ParamsForLoginKey;
						NewParams.LocalPlayerIndex = NextLocalPlayerIndex;
						NewParams.PrimaryInputDevice = EventArgs.InputDevice;

						return TryToInitializeUser(NewParams);
					}
				}
			}
		}
	}

	if (WrappedInputKeyHandler.IsBound())
	{
		return WrappedInputKeyHandler.Execute(EventArgs);
	}

	return false;
}

static inline FText GetErrorText(const FOnlineErrorType& InOnlineError)
{
	return InOnlineError.GetErrorMessage();
}

const UGameUserInfo* UGameUserSubsystem::GetUserInfoForLocalPlayerIndex(int32 LocalPlayerIndex) const
{
	TObjectPtr<UGameUserInfo> const* Found = LocalUserInfos.Find(LocalPlayerIndex);
	if (Found)
	{
		return *Found;
	}
	return nullptr;
}

const UGameUserInfo* UGameUserSubsystem::GetUserInfoForPlatformUserIndex(int32 PlatformUserIndex) const
{
	FPlatformUserId PlatformUser = GetPlatformUserIdForIndex(PlatformUserIndex);
	return GetUserInfoForPlatformUser(PlatformUser);
}

const UGameUserInfo* UGameUserSubsystem::GetUserInfoForPlatformUser(FPlatformUserId PlatformUser) const
{
	if (!IsRealPlatformUser(PlatformUser))
	{
		return nullptr;
	}

	for (TPair<int32, UGameUserInfo*> Pair : LocalUserInfos)
	{
		// 此检查中不包含游客用户
		if (ensure(Pair.Value) && Pair.Value->PlatformUser == PlatformUser && !Pair.Value->bIsGuest)
		{
			return Pair.Value;
		}
	}

	return nullptr;
}

const UGameUserInfo* UGameUserSubsystem::GetUserInfoForUniqueNetId(const FUniqueNetIdRepl& NetId) const
{
	if (!NetId.IsValid())
	{
		// TODO 在移动平台上,netID 无效的登录前情况是否需要处理?
		return nullptr;
	}

	for (TPair<int32, UGameUserInfo*> UserPair : LocalUserInfos)
	{
		if (ensure(UserPair.Value))
		{
			for (const TPair<EGameUserOnlineContext, UGameUserInfo::FCachedData>& CachedPair : UserPair.Value->CachedDataMap)
			{
				if (NetId == CachedPair.Value.CachedNetId)
				{
					return UserPair.Value;
				}
			}
		}
	}

	return nullptr;
}

const UGameUserInfo* UGameUserSubsystem::GetUserInfoForControllerId(int32 ControllerId) const
{
	FPlatformUserId PlatformUser;
	FInputDeviceId IgnoreDevice;

	IPlatformInputDeviceMapper::Get().RemapControllerIdToPlatformUserAndDevice(ControllerId, PlatformUser, IgnoreDevice);

	return GetUserInfoForPlatformUser(PlatformUser);
}

const UGameUserInfo* UGameUserSubsystem::GetUserInfoForInputDevice(FInputDeviceId InputDevice) const
{
	FPlatformUserId PlatformUser = GetPlatformUserIdForInputDevice(InputDevice);
	return GetUserInfoForPlatformUser(PlatformUser);
}

bool UGameUserSubsystem::IsRealPlatformUserIndex(int32 PlatformUserIndex) const
{
	if (PlatformUserIndex < 0)
	{
		return false;
	}

	if (PlatformUserIndex >= MAX_LOCAL_PLAYERS)
	{
		// 对照 OSS 的玩家数量进行检查
		return false;
	}

	if (PlatformUserIndex > 0 && GetTraitTags().HasTag(FGameUserTags::TAG_Platform_Trait_SingleOnlineUser))
	{
		return false;
	}

	return true;
}

bool UGameUserSubsystem::IsRealPlatformUser(FPlatformUserId PlatformUser) const
{
	// 验证已在转换/分配时完成,因此信任该类型
	if (!PlatformUser.IsValid())
	{
		return false;
	}

	// TODO: 以某种方式对照 OSS 或输入映射器进行验证

	if (GetTraitTags().HasTag(FGameUserTags::TAG_Platform_Trait_SingleOnlineUser))
	{
		// 只有默认用户支持在线功能
		if (PlatformUser != IPlatformInputDeviceMapper::Get().GetPrimaryPlatformUser())
		{
			return false;
		}
	}

	return true;
}

FPlatformUserId UGameUserSubsystem::GetPlatformUserIdForIndex(int32 PlatformUserIndex) const
{
	return IPlatformInputDeviceMapper::Get().GetPlatformUserForUserIndex(PlatformUserIndex);
}

int32 UGameUserSubsystem::GetPlatformUserIndexForId(FPlatformUserId PlatformUser) const
{
	return IPlatformInputDeviceMapper::Get().GetUserIndexForPlatformUser(PlatformUser);
}

FPlatformUserId UGameUserSubsystem::GetPlatformUserIdForInputDevice(FInputDeviceId InputDevice) const
{
	return IPlatformInputDeviceMapper::Get().GetUserForInputDevice(InputDevice);
}

FInputDeviceId UGameUserSubsystem::GetPrimaryInputDeviceForPlatformUser(FPlatformUserId PlatformUser) const
{
	return IPlatformInputDeviceMapper::Get().GetPrimaryInputDeviceForUser(PlatformUser);
}

void UGameUserSubsystem::SetTraitTags(const FGameplayTagContainer& InTags)
{
	CachedTraitTags = InTags;
}

bool UGameUserSubsystem::ShouldWaitForStartInput() const
{
	// 默认情况下,如果是单用户平台,则不等待输入
	return !HasTraitTag(FGameUserTags::TAG_Platform_Trait_SingleOnlineUser.GetTag());
}

FString UGameUserSubsystem::PlatformUserIdToString(FPlatformUserId UserId)
{
	if (UserId == PLATFORMUSERID_NONE)
	{
		return TEXT("None");
	}
	else
	{
		return FString::Printf(TEXT("%d"), UserId.GetInternalId());
	}
}

FString UGameUserSubsystem::EGameUserOnlineContextToString(EGameUserOnlineContext Context)
{
	switch (Context)
	{
	case EGameUserOnlineContext::Game:
		return TEXT("Game");
	case EGameUserOnlineContext::Default:
		return TEXT("Default");
	case EGameUserOnlineContext::Service:
		return TEXT("Service");
	case EGameUserOnlineContext::ServiceOrDefault:
		return TEXT("Service/Default");
	case EGameUserOnlineContext::Platform:
		return TEXT("Platform");
	case EGameUserOnlineContext::PlatformOrDefault:
		return TEXT("Platform/Default");
	default:
		return TEXT("Invalid");
	}
}

FText UGameUserSubsystem::GetPrivilegeDescription(EGameUserPrivilege Privilege) const
{
	switch (Privilege)
	{
	case EGameUserPrivilege::CanPlay:
		return NSLOCTEXT("GameUser", "PrivilegeCanPlay", "play the game");
	case EGameUserPrivilege::CanPlayOnline:
		return NSLOCTEXT("GameUser", "PrivilegeCanPlayOnline", "play online");
	case EGameUserPrivilege::CanCommunicateViaTextOnline:
		return NSLOCTEXT("GameUser", "PrivilegeCanCommunicateViaTextOnline", "communicate with text");
	case EGameUserPrivilege::CanCommunicateViaVoiceOnline:
		return NSLOCTEXT("GameUser", "PrivilegeCanCommunicateViaVoiceOnline", "communicate with voice");
	case EGameUserPrivilege::CanUseUserGeneratedContent:
		return NSLOCTEXT("GameUser", "PrivilegeCanUseUserGeneratedContent", "access user content");
	case EGameUserPrivilege::CanUseCrossPlay:
		return NSLOCTEXT("GameUser", "PrivilegeCanUseCrossPlay", "play with other platforms");
	default:
		return NSLOCTEXT("GameUser", "PrivilegeInvalid", "Invalid");
	}
}

FText UGameUserSubsystem::GetPrivilegeResultDescription(EGameUserPrivilegeResult Result) const
{
	// TODO 这些字符串可能有认证要求,我们需要按主机平台覆盖
	switch (Result)
	{
	case EGameUserPrivilegeResult::Unknown:
		return NSLOCTEXT("GameUser", "ResultUnknown", "Unknown if the user is allowed");
	case EGameUserPrivilegeResult::Available:
		return NSLOCTEXT("GameUser", "ResultAvailable", "The user is allowed");
	case EGameUserPrivilegeResult::UserNotLoggedIn:
		return NSLOCTEXT("GameUser", "ResultUserNotLoggedIn", "The user must login");
	case EGameUserPrivilegeResult::LicenseInvalid:
		return NSLOCTEXT("GameUser", "ResultLicenseInvalid", "A valid game license is required");
	case EGameUserPrivilegeResult::VersionOutdated:
		return NSLOCTEXT("GameUser", "VersionOutdated", "The game or hardware needs to be updated");
	case EGameUserPrivilegeResult::NetworkConnectionUnavailable:
		return NSLOCTEXT("GameUser", "ResultNetworkConnectionUnavailable", "A network connection is required");
	case EGameUserPrivilegeResult::AgeRestricted:
		return NSLOCTEXT("GameUser", "ResultAgeRestricted", "This age restricted account is not allowed");
	case EGameUserPrivilegeResult::AccountTypeRestricted:
		return NSLOCTEXT("GameUser", "ResultAccountTypeRestricted", "This account type does not have access");
	case EGameUserPrivilegeResult::AccountUseRestricted:
		return NSLOCTEXT("GameUser", "ResultAccountUseRestricted", "This account is not allowed");
	case EGameUserPrivilegeResult::PlatformFailure:
		return NSLOCTEXT("GameUser", "ResultPlatformFailure", "Not allowed");
	default:
		return NSLOCTEXT("GameUser", "ResultInvalid", "Invalid");
	}
}

bool UGameUserSubsystem::LoginLocalUser(const UGameUserInfo* UserInfo, EGameUserPrivilege RequestedPrivilege, EGameUserOnlineContext Context, FOnLocalUserLoginCompleteDelegate OnComplete)
{
	UGameUserInfo* LocalUserInfo = ModifyInfo(UserInfo);
	if (!ensure(UserInfo))
	{
		return false;
	}

	TSharedRef<FUserLoginRequest> NewRequest = MakeShared<FUserLoginRequest>(LocalUserInfo, RequestedPrivilege, Context, MoveTemp(OnComplete));
	ActiveLoginRequests.Add(NewRequest);

	// 这将执行回调或启动登录流程
	ProcessLoginRequest(NewRequest);

	return true;
}

void UGameUserSubsystem::SetLocalPlayerUserInfo(ULocalPlayer* LocalPlayer, const UGameUserInfo* UserInfo)
{
	if (!bIsDedicatedServer && ensure(LocalPlayer && UserInfo))
	{
		LocalPlayer->SetPlatformUserId(UserInfo->GetPlatformUserId());

		FUniqueNetIdRepl NetId = UserInfo->GetNetId(EGameUserOnlineContext::Game);
		LocalPlayer->SetCachedUniqueNetId(NetId);

		// 如有可能,同时更新玩家状态(PlayerState)
		APlayerController* PlayerController = LocalPlayer->GetPlayerController(nullptr);
		if (PlayerController && PlayerController->PlayerState)
		{
			PlayerController->PlayerState->SetUniqueId(NetId);
		}
	}
}

EGameUserOnlineContext UGameUserSubsystem::ResolveOnlineContext(EGameUserOnlineContext Context) const
{
	switch (Context)
	{
	case EGameUserOnlineContext::Game:
	case EGameUserOnlineContext::Default:
		return EGameUserOnlineContext::Default;

	case EGameUserOnlineContext::Service:
		return ServiceContextInternal ? EGameUserOnlineContext::Service : EGameUserOnlineContext::Invalid;
	case EGameUserOnlineContext::ServiceOrDefault:
		return ServiceContextInternal ? EGameUserOnlineContext::Service : EGameUserOnlineContext::Default;

	case EGameUserOnlineContext::Platform:
		return PlatformContextInternal ? EGameUserOnlineContext::Platform : EGameUserOnlineContext::Invalid;
	case EGameUserOnlineContext::PlatformOrDefault:
		return PlatformContextInternal ? EGameUserOnlineContext::Platform : EGameUserOnlineContext::Default;
	}

	return EGameUserOnlineContext::Invalid;
}

bool UGameUserSubsystem::HasSeparatePlatformContext() const
{
	EGameUserOnlineContext ServiceType = ResolveOnlineContext(EGameUserOnlineContext::ServiceOrDefault);
	EGameUserOnlineContext PlatformType = ResolveOnlineContext(EGameUserOnlineContext::PlatformOrDefault);

	if (ServiceType != PlatformType)
	{
		return true;
	}
	return false;
}

void UGameUserSubsystem::RefreshLocalUserInfo(UGameUserInfo* UserInfo)
{
	if (ensure(UserInfo))
	{
		// 始终更新默认(Default)缓存
		UserInfo->UpdateCachedNetId(GetLocalUserNetId(UserInfo->PlatformUser, EGameUserOnlineContext::Default), EGameUserOnlineContext::Default);

		if (HasSeparatePlatformContext())
		{
			// 同时更新平台(Platform)缓存
			UserInfo->UpdateCachedNetId(GetLocalUserNetId(UserInfo->PlatformUser, EGameUserOnlineContext::Platform), EGameUserOnlineContext::Platform);
		}
	}
}

void UGameUserSubsystem::HandleChangedAvailability(UGameUserInfo* UserInfo, EGameUserPrivilege Privilege, EGameUserAvailability OldAvailability)
{
	EGameUserAvailability NewAvailability = UserInfo->GetPrivilegeAvailability(Privilege);

	if (OldAvailability != NewAvailability)
	{
		OnUserPrivilegeChanged.Broadcast(UserInfo, Privilege, OldAvailability, NewAvailability);
	}
}

void UGameUserSubsystem::UpdateUserPrivilegeResult(UGameUserInfo* UserInfo, EGameUserPrivilege Privilege, EGameUserPrivilegeResult Result, EGameUserOnlineContext Context)
{
	check(UserInfo);

	EGameUserAvailability OldAvailability = UserInfo->GetPrivilegeAvailability(Privilege);

	UserInfo->UpdateCachedPrivilegeResult(Privilege, Result, Context);

	HandleChangedAvailability(UserInfo, Privilege, OldAvailability);
}

const UGameUserSubsystem::FOnlineContextCache* UGameUserSubsystem::GetContextCache(EGameUserOnlineContext Context) const
{
	return const_cast<UGameUserSubsystem*>(this)->GetContextCache(Context);
}

UGameUserSubsystem::FOnlineContextCache* UGameUserSubsystem::GetContextCache(EGameUserOnlineContext Context)
{
	switch (Context)
	{
	case EGameUserOnlineContext::Game:
	case EGameUserOnlineContext::Default:
		return DefaultContextInternal;

	case EGameUserOnlineContext::Service:
		return ServiceContextInternal;
	case EGameUserOnlineContext::ServiceOrDefault:
		return ServiceContextInternal ? ServiceContextInternal : DefaultContextInternal;

	case EGameUserOnlineContext::Platform:
		return PlatformContextInternal;
	case EGameUserOnlineContext::PlatformOrDefault:
		return PlatformContextInternal ? PlatformContextInternal : DefaultContextInternal;
	}

	return nullptr;
}

void UGameUserSubsystem::ProcessLoginRequest(TSharedRef<FUserLoginRequest> Request)
{
	// 首先,检查我们是否已完全登录
	UGameUserInfo* UserInfo = Request->UserInfo.Get();

	if (!UserInfo)
	{
		// 用户已消失,直接删除此请求
		ActiveLoginRequests.Remove(Request);

		return;
	}

	const FPlatformUserId PlatformUser = UserInfo->GetPlatformUserId();

	// 如果因游客身份导致平台用户 ID 无效,直接跳到失败处理
	if (!IsRealPlatformUser(PlatformUser))
	{
		Request->Error = FOnlineError(NSLOCTEXT("GameUser", "InvalidPlatformUser", "Invalid Platform User"));
		// 从活动请求数组中移除
		ActiveLoginRequests.Remove(Request);

		// 如果委托已绑定,则执行
		Request->Delegate.ExecuteIfBound(UserInfo, ELoginStatusType::NotLoggedIn, FUniqueNetIdRepl(), Request->Error, Request->DesiredContext);

		return;
	}

	// 确定首先处理哪个上下文
	if (Request->CurrentContext == EGameUserOnlineContext::Invalid)
	{
		// 如果是游戏(Game)登录,首先从平台上下文开始
		if (Request->DesiredContext == EGameUserOnlineContext::Game)
		{
			Request->CurrentContext = ResolveOnlineContext(EGameUserOnlineContext::PlatformOrDefault);
		}
		else
		{
			Request->CurrentContext = ResolveOnlineContext(Request->DesiredContext);
		}
	}

	ELoginStatusType CurrentStatus = GetLocalUserLoginStatus(PlatformUser, Request->CurrentContext);
	FUniqueNetIdRepl CurrentId = GetLocalUserNetId(PlatformUser, Request->CurrentContext);
	FOnlineContextCache* System = GetContextCache(Request->CurrentContext);

	if (!ensure(System))
	{
		return;
	}

	// 开始一个新请求
	if (Request->OverallLoginState == EGameUserAsyncTaskState::NotStarted)
	{
		Request->OverallLoginState = EGameUserAsyncTaskState::InProgress;
	}

	bool bHasRequiredStatus = (CurrentStatus == ELoginStatusType::LoggedIn);
	if (Request->DesiredPrivilege == EGameUserPrivilege::CanPlay)
	{
		// 如果这不是必须联网的登录,允许本地档案视为完全登录
		bHasRequiredStatus |= (CurrentStatus == ELoginStatusType::UsingLocalProfile);
	}

	// 检查整体是否成功
	if (bHasRequiredStatus && CurrentId.IsValid())
	{
		// 如果正在等待登录 UI 关闭,则暂停
		if (Request->LoginUIState == EGameUserAsyncTaskState::InProgress)
		{
			return;
		}

		Request->OverallLoginState = EGameUserAsyncTaskState::Done;
	}
	else
	{
		// 尝试使用平台认证登录
		if (Request->TransferPlatformAuthState == EGameUserAsyncTaskState::NotStarted)
		{
			Request->TransferPlatformAuthState = EGameUserAsyncTaskState::InProgress;

			if (TransferPlatformAuth(System, Request, PlatformUser))
			{
				return;
			}
			// 我们没有发起登录尝试,因此设为失败
			Request->TransferPlatformAuthState = EGameUserAsyncTaskState::Failed;
		}

		// 接下来检查自动登录(AutoLogin)
		if (Request->AutoLoginState == EGameUserAsyncTaskState::NotStarted)
		{
			if (Request->TransferPlatformAuthState == EGameUserAsyncTaskState::Done || Request->TransferPlatformAuthState == EGameUserAsyncTaskState::Failed)
			{
				Request->AutoLoginState = EGameUserAsyncTaskState::InProgress;

				// 尝试使用默认凭据自动登录,这在许多平台上都有效
				if (AutoLogin(System, Request, PlatformUser))
				{
					return;
				}
				// 我们没有发起自动登录尝试,因此设为失败
				Request->AutoLoginState = EGameUserAsyncTaskState::Failed;
			}
		}

		// 接下来检查登录 UI
		if (Request->LoginUIState == EGameUserAsyncTaskState::NotStarted)
		{
			if ((Request->TransferPlatformAuthState == EGameUserAsyncTaskState::Done || Request->TransferPlatformAuthState == EGameUserAsyncTaskState::Failed)
				&& (Request->AutoLoginState == EGameUserAsyncTaskState::Done || Request->AutoLoginState == EGameUserAsyncTaskState::Failed))
			{
				Request->LoginUIState = EGameUserAsyncTaskState::InProgress;

				if (ShowLoginUI(System, Request, PlatformUser))
				{
					return;
				}
				// 我们没有显示 UI,因此设为失败
				Request->LoginUIState = EGameUserAsyncTaskState::Failed;
			}
		}
	}

	// 检查整体是否失败
	if (Request->LoginUIState == EGameUserAsyncTaskState::Failed &&
		Request->AutoLoginState == EGameUserAsyncTaskState::Failed &&
		Request->TransferPlatformAuthState == EGameUserAsyncTaskState::Failed)
	{
		Request->OverallLoginState = EGameUserAsyncTaskState::Failed;
	}
	else if (Request->OverallLoginState == EGameUserAsyncTaskState::InProgress &&
		Request->LoginUIState != EGameUserAsyncTaskState::InProgress &&
		Request->AutoLoginState != EGameUserAsyncTaskState::InProgress &&
		Request->TransferPlatformAuthState != EGameUserAsyncTaskState::InProgress)
	{
		// 如果所有子状态都不再处于进行中,但我们尚未成功登录,则标记为失败,以避免永久卡住
		Request->OverallLoginState = EGameUserAsyncTaskState::Failed;
	}

	if (Request->OverallLoginState == EGameUserAsyncTaskState::Done)
	{
		// 如有需要,执行权限检查
		if (Request->PrivilegeCheckState == EGameUserAsyncTaskState::NotStarted)
		{
			Request->PrivilegeCheckState = EGameUserAsyncTaskState::InProgress;

			EGameUserPrivilegeResult CachedResult = UserInfo->GetCachedPrivilegeResult(Request->DesiredPrivilege, Request->CurrentContext);
			if (CachedResult == EGameUserPrivilegeResult::Available)
			{
				// 使用缓存的成功结果
				Request->PrivilegeCheckState = EGameUserAsyncTaskState::Done;
			}
			else
			{
				if (QueryUserPrivilege(System, Request, PlatformUser))
				{
					return;
				}
				else
				{
				}
			}
		}

		if (Request->PrivilegeCheckState == EGameUserAsyncTaskState::Failed)
		{
			// 将权限失败计为登录失败
			Request->OverallLoginState = EGameUserAsyncTaskState::Failed;
		}
		else if (Request->PrivilegeCheckState == EGameUserAsyncTaskState::Done)
		{
			// 如果平台上下文已完成,但仍需处理服务上下文,则接下来处理
			EGameUserOnlineContext ResolvedDesiredContext = ResolveOnlineContext(Request->DesiredContext);

			if (Request->OverallLoginState == EGameUserAsyncTaskState::Done && Request->CurrentContext != ResolvedDesiredContext)
			{
				Request->CurrentContext = ResolvedDesiredContext;
				Request->OverallLoginState = EGameUserAsyncTaskState::NotStarted;
				Request->PrivilegeCheckState = EGameUserAsyncTaskState::NotStarted;
				Request->TransferPlatformAuthState = EGameUserAsyncTaskState::NotStarted;
				Request->AutoLoginState = EGameUserAsyncTaskState::NotStarted;
				Request->LoginUIState = EGameUserAsyncTaskState::NotStarted;

				// 重新处理并立即返回
				ProcessLoginRequest(Request);
				return;
			}
		}
	}

	if (Request->PrivilegeCheckState == EGameUserAsyncTaskState::InProgress)
	{
		// 暂停以等待其完成
		return;
	}

	// 如果已完成,移除并执行回调
	if (Request->OverallLoginState == EGameUserAsyncTaskState::Done || Request->OverallLoginState == EGameUserAsyncTaskState::Failed)
	{
		// 如果这在嵌套函数中已处理过,则跳过
		if (ActiveLoginRequests.Contains(Request))
		{
			// 如果未设置错误,则添加一个通用错误
			if (Request->OverallLoginState == EGameUserAsyncTaskState::Failed && !Request->Error.IsSet())
			{
				Request->Error = FOnlineError(NSLOCTEXT("GameUser", "FailedToRequest", "Failed to Request Login"));
			}

			// 从活动请求数组中移除
			ActiveLoginRequests.Remove(Request);

			// 如果委托已绑定,则执行
			Request->Delegate.ExecuteIfBound(UserInfo, CurrentStatus, CurrentId, Request->Error, Request->DesiredContext);
		}
	}
}

EGameUserPrivilege UGameUserSubsystem::ConvertOSSPrivilege(EUserPrivileges::Type Privilege) const
{
	switch (Privilege)
	{
	case EUserPrivileges::CanPlay:
		return EGameUserPrivilege::CanPlay;
	case EUserPrivileges::CanPlayOnline:
		return EGameUserPrivilege::CanPlayOnline;
	case EUserPrivileges::CanCommunicateOnline:
		return EGameUserPrivilege::CanCommunicateViaTextOnline; // 这里没有更好的处理方式,只能映射为文本(Text)权限。
	case EUserPrivileges::CanUseUserGeneratedContent:
		return EGameUserPrivilege::CanUseUserGeneratedContent;
	case EUserPrivileges::CanUserCrossPlay:
		return EGameUserPrivilege::CanUseCrossPlay;
	default:
		return EGameUserPrivilege::Invalid_Count;
	}
}

EUserPrivileges::Type UGameUserSubsystem::ConvertOSSPrivilege(EGameUserPrivilege Privilege) const
{
	switch (Privilege)
	{
	case EGameUserPrivilege::CanPlay:
		return EUserPrivileges::CanPlay;
	case EGameUserPrivilege::CanPlayOnline:
		return EUserPrivileges::CanPlayOnline;
	case EGameUserPrivilege::CanCommunicateViaTextOnline:
	case EGameUserPrivilege::CanCommunicateViaVoiceOnline:
		return EUserPrivileges::CanCommunicateOnline;
	case EGameUserPrivilege::CanUseUserGeneratedContent:
		return EUserPrivileges::CanUseUserGeneratedContent;
	case EGameUserPrivilege::CanUseCrossPlay:
		return EUserPrivileges::CanUserCrossPlay;
	default:
		// 没有失败类型,返回 CanPlay
		return EUserPrivileges::CanPlay;
	}
}

EGameUserPrivilegeResult UGameUserSubsystem::ConvertOSSPrivilegeResult(EUserPrivileges::Type Privilege, uint32 Results) const
{
	// V1 结果枚举是按位标志(bitfield),每个平台的表现略有不同
	if (Results == (uint32)IOnlineIdentity::EPrivilegeResults::NoFailures)
	{
		return EGameUserPrivilegeResult::Available;
	}
	if ((Results & (uint32)IOnlineIdentity::EPrivilegeResults::UserNotFound) || (Results & (uint32)IOnlineIdentity::EPrivilegeResults::UserNotLoggedIn))
	{
		return EGameUserPrivilegeResult::UserNotLoggedIn;
	}
	if ((Results & (uint32)IOnlineIdentity::EPrivilegeResults::RequiredPatchAvailable) || (Results & (uint32)IOnlineIdentity::EPrivilegeResults::RequiredSystemUpdate))
	{
		return EGameUserPrivilegeResult::VersionOutdated;
	}
	if (Results & (uint32)IOnlineIdentity::EPrivilegeResults::AgeRestrictionFailure)
	{
		return EGameUserPrivilegeResult::AgeRestricted;
	}
	if (Results & (uint32)IOnlineIdentity::EPrivilegeResults::AccountTypeFailure)
	{
		return EGameUserPrivilegeResult::AccountTypeRestricted;
	}
	if (Results & (uint32)IOnlineIdentity::EPrivilegeResults::NetworkConnectionUnavailable)
	{
		return EGameUserPrivilegeResult::NetworkConnectionUnavailable;
	}

	// 将其他账户失败情况归为一类
	uint32 AccountUseFailures = (uint32)IOnlineIdentity::EPrivilegeResults::OnlinePlayRestricted
		| (uint32)IOnlineIdentity::EPrivilegeResults::UGCRestriction
		| (uint32)IOnlineIdentity::EPrivilegeResults::ChatRestriction;

	if (Results & AccountUseFailures)
	{
		return EGameUserPrivilegeResult::AccountUseRestricted;
	}

	// 如果完全无法游玩,这属于许可证失败
	if (Privilege == EUserPrivileges::CanPlay)
	{
		return EGameUserPrivilegeResult::LicenseInvalid;
	}

	// 未知原因
	return EGameUserPrivilegeResult::PlatformFailure;
}

void UGameUserSubsystem::HandleIdentityLoginStatusChanged(int32 PlatformUserIndex, ELoginStatus::Type OldStatus, ELoginStatus::Type NewStatus, const FUniqueNetId& NewId, EGameUserOnlineContext Context)
{
	UE_LOG(LogGameUser, Log, TEXT("Player login status changed - System:%s, UserIdx:%d, OldStatus:%s, NewStatus:%s, NewId:%s"),
	       *GetOnlineSubsystemName(Context).ToString(),
	       PlatformUserIndex,
	       ELoginStatus::ToString(OldStatus),
	       ELoginStatus::ToString(NewStatus),
	       *NewId.ToString());

	if (NewStatus == ELoginStatus::NotLoggedIn && OldStatus != ELoginStatus::NotLoggedIn)
	{
		FPlatformUserId PlatformUser = GetPlatformUserIdForIndex(PlatformUserIndex);
		LogOutLocalUser(PlatformUser);
	}
}

void UGameUserSubsystem::HandleUserLoginCompleted(int32 PlatformUserIndex, bool bWasSuccessful, const FUniqueNetId& NetId, const FString& ErrorString, EGameUserOnlineContext Context)
{
	FPlatformUserId PlatformUser = GetPlatformUserIdForIndex(PlatformUserIndex);
	ELoginStatusType NewStatus = GetLocalUserLoginStatus(PlatformUser, Context);
	FUniqueNetIdRepl NewId = FUniqueNetIdRepl(NetId);
	UE_LOG(LogGameUser, Log, TEXT("Player login Completed - System:%s, UserIdx:%d, Successful:%d, NewStatus:%s, NewId:%s, ErrorIfAny:%s"),
	       *GetOnlineSubsystemName(Context).ToString(),
	       PlatformUserIndex,
	       (int32)bWasSuccessful,
	       ELoginStatus::ToString(NewStatus),
	       *NewId.ToString(),
	       *ErrorString);

	// 更新所有等待中的登录请求
	TArray<TSharedRef<FUserLoginRequest>> RequestsCopy = ActiveLoginRequests;
	for (TSharedRef<FUserLoginRequest>& Request : RequestsCopy)
	{
		UGameUserInfo* UserInfo = Request->UserInfo.Get();

		if (!UserInfo)
		{
			// 用户已消失,直接删除此请求
			ActiveLoginRequests.Remove(Request);

			continue;
		}

		if (UserInfo->PlatformUser == PlatformUser && Request->CurrentContext == Context)
		{
			// 在某些平台上,登录 UI 会以失败状态调用此函数
			if (Request->AutoLoginState == EGameUserAsyncTaskState::InProgress)
			{
				Request->AutoLoginState = bWasSuccessful ? EGameUserAsyncTaskState::Done : EGameUserAsyncTaskState::Failed;
			}

			if (!bWasSuccessful)
			{
				Request->Error = FOnlineError(FText::FromString(ErrorString));
			}

			ProcessLoginRequest(Request);
		}
	}
}

void UGameUserSubsystem::HandleControllerPairingChanged(int32 PlatformUserIndex, FControllerPairingChangedUserInfo PreviousUser, FControllerPairingChangedUserInfo NewUser)
{
	UE_LOG(LogGameUser, Log, TEXT("Player controller pairing changed - UserIdx:%d, PreviousUser:%s, NewUser:%s"),
	       PlatformUserIndex,
	       *ToDebugString(PreviousUser),
	       *ToDebugString(NewUser));

	UGameInstance* GameInstance = GetGameInstance();
	FPlatformUserId PlatformUser = GetPlatformUserIdForIndex(PlatformUserIndex);
	ULocalPlayer* ControlledLocalPlayer = GameInstance->FindLocalPlayerFromPlatformUserId(PlatformUser);
	ULocalPlayer* NewLocalPlayer = GameInstance->FindLocalPlayerFromUniqueNetId(NewUser.User);
	const UGameUserInfo* NewUserInfo = GetUserInfoForUniqueNetId(FUniqueNetIdRepl(NewUser.User));
	const UGameUserInfo* PreviousUserInfo = GetUserInfoForUniqueNetId(FUniqueNetIdRepl(PreviousUser.User));

	// 检查我们是否认为该用户已绑定到现有玩家
	if (PreviousUser.ControllersRemaining == 0 && PreviousUserInfo && PreviousUserInfo != NewUserInfo)
	{
		// 这意味着用户通过平台界面主动登出
		if (IsRealPlatformUser(PlatformUser))
		{
			LogOutLocalUser(PlatformUser);
		}
	}

	if (ControlledLocalPlayer && ControlledLocalPlayer != NewLocalPlayer)
	{
		// TODO 目前,调用此委托的平台并未真正处理手柄 ID 的交换
		// SetLocalPlayerUserIndex(ControlledLocalPlayer, -1);
	}
}

void UGameUserSubsystem::HandleNetworkConnectionStatusChanged(const FString& ServiceName, EOnlineServerConnectionStatus::Type LastConnectionStatus, EOnlineServerConnectionStatus::Type ConnectionStatus, EGameUserOnlineContext Context)
{
	UE_LOG(LogGameUser, Log, TEXT("HandleNetworkConnectionStatusChanged(ServiceName: %s, LastStatus: %s, ConnectionStatus: %s)"),
	       *ServiceName,
	       EOnlineServerConnectionStatus::ToString(LastConnectionStatus),
	       EOnlineServerConnectionStatus::ToString(ConnectionStatus));

	// 缓存当前用户的旧可用性
	TMap<UGameUserInfo*, EGameUserAvailability> AvailabilityMap;

	for (TPair<int32, UGameUserInfo*> Pair : LocalUserInfos)
	{
		AvailabilityMap.Add(Pair.Value, Pair.Value->GetPrivilegeAvailability(EGameUserPrivilege::CanPlayOnline));
	}

	FOnlineContextCache* System = GetContextCache(Context);
	if (ensure(System))
	{
		// 服务名称通常与 OSS 名称相同,但在某些平台上不一定如此
		System->CurrentConnectionStatus = ConnectionStatus;
	}

	for (TPair<UGameUserInfo*, EGameUserAvailability> Pair : AvailabilityMap)
	{
		// 当有人上线/下线时通知其他系统
		HandleChangedAvailability(Pair.Key, EGameUserPrivilege::CanPlayOnline, Pair.Value);
	}
}

void UGameUserSubsystem::HandleOnLoginUIClosed(TSharedPtr<const FUniqueNetId> LoggedInNetId, const int PlatformUserIndex, const FOnlineError& Error, EGameUserOnlineContext Context)
{
	FPlatformUserId PlatformUser = GetPlatformUserIdForIndex(PlatformUserIndex);

	// 更新所有等待中的登录请求
	TArray<TSharedRef<FUserLoginRequest>> RequestsCopy = ActiveLoginRequests;
	for (TSharedRef<FUserLoginRequest>& Request : RequestsCopy)
	{
		UGameUserInfo* UserInfo = Request->UserInfo.Get();

		if (!UserInfo)
		{
			// 用户已消失,直接删除此请求
			ActiveLoginRequests.Remove(Request);

			continue;
		}

		// 查找第一个在此上下文上尝试登录的用户
		if (Request->CurrentContext == Context && Request->LoginUIState == EGameUserAsyncTaskState::InProgress)
		{
			if (LoggedInNetId.IsValid() && LoggedInNetId->IsValid() && Error.WasSuccessful())
			{
				// 实际登录的平台用户 ID 可能与请求 UI 的用户不同,
				// 因此,如果返回的 ID 确实有效,则进行替换
				if (UserInfo->PlatformUser != PlatformUser && PlatformUser != PLATFORMUSERID_NONE)
				{
					UserInfo->PlatformUser = PlatformUser;
				}

				Request->LoginUIState = EGameUserAsyncTaskState::Done;
				Request->Error.Reset();
			}
			else
			{
				Request->LoginUIState = EGameUserAsyncTaskState::Failed;
				Request->Error = Error;
			}

			ProcessLoginRequest(Request);
		}
	}
}

void UGameUserSubsystem::HandleCheckPrivilegesComplete(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults, EGameUserPrivilege UserPrivilege, TWeakObjectPtr<UGameUserInfo> GameUserInfo,
                                                       EGameUserOnlineContext Context)
{
	// 仅在用户仍然存在时处理
	UGameUserInfo* UserInfo = GameUserInfo.Get();

	if (!UserInfo)
	{
		return;
	}

	EGameUserPrivilegeResult UserResult = ConvertOSSPrivilegeResult(Privilege, PrivilegeResults);

	// 更新用户缓存的值
	UpdateUserPrivilegeResult(UserInfo, UserPrivilege, UserResult, Context);

	FOnlineContextCache* ContextCache = GetContextCache(Context);
	check(ContextCache);

	// 如果返回断开连接状态,则更新连接状态
	if (UserResult == EGameUserPrivilegeResult::NetworkConnectionUnavailable)
	{
		ContextCache->CurrentConnectionStatus = EOnlineServerConnectionStatus::NoNetworkConnection;
	}
	else if (UserResult == EGameUserPrivilegeResult::Available && UserPrivilege == EGameUserPrivilege::CanPlayOnline)
	{
		if (ContextCache->CurrentConnectionStatus == EOnlineServerConnectionStatus::NoNetworkConnection)
		{
			ContextCache->CurrentConnectionStatus = EOnlineServerConnectionStatus::Normal;
		}
	}

	// 查看是否有登录请求正在等待此结果
	TArray<TSharedRef<FUserLoginRequest>> RequestsCopy = ActiveLoginRequests;
	for (TSharedRef<FUserLoginRequest>& Request : RequestsCopy)
	{
		if (Request->UserInfo.Get() == UserInfo && Request->CurrentContext == Context && Request->DesiredPrivilege == UserPrivilege && Request->PrivilegeCheckState == EGameUserAsyncTaskState::InProgress)
		{
			if (UserResult == EGameUserPrivilegeResult::Available)
			{
				Request->PrivilegeCheckState = EGameUserAsyncTaskState::Done;
			}
			else
			{
				Request->PrivilegeCheckState = EGameUserAsyncTaskState::Failed;

				// 生成类似 "(The user is not allowed) to (play the game)" 的英文字符串
				Request->Error = FOnlineError(FText::Format(NSLOCTEXT("GameUser", "PrivilegeFailureFormat", "{0} to {1}"), GetPrivilegeResultDescription(UserResult), GetPrivilegeDescription(UserPrivilege)));
			}

			ProcessLoginRequest(Request);
		}
	}
}

void UGameUserSubsystem::HandleInputDeviceConnectionChanged(EInputDeviceConnectionState NewConnectionState, FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId)
{
	FString InputDeviceIDString = FString::Printf(TEXT("%d"), InputDeviceId.GetId());
	const bool bIsConnected = NewConnectionState == EInputDeviceConnectionState::Connected;
	UE_LOG(LogGameUser, Log, TEXT("Controller connection changed - UserIdx:%s, UserID:%s, Connected:%d"), *InputDeviceIDString, *PlatformUserIdToString(PlatformUserId), bIsConnected ? 1 : 0);

	// TODO 为支持此功能的平台实现相应处理
}

void UGameUserSubsystem::HandleLoginForUserInitialize(const UGameUserInfo* UserInfo, ELoginStatusType NewStatus, FUniqueNetIdRepl NetId, const TOptional<FOnlineErrorType>& InError, EGameUserOnlineContext Context,
                                                      FGameUserInitializeParams Params)
{
	UGameInstance* GameInstance = GetGameInstance();
	check(GameInstance);
	FTimerManager& TimerManager = GameInstance->GetTimerManager();
	TOptional<FOnlineErrorType> Error = InError; // 复制一份,以便在错误被处理时能够重置

	UGameUserInfo* LocalUserInfo = ModifyInfo(UserInfo);
	UGameUserInfo* FirstUserInfo = ModifyInfo(GetUserInfoForLocalPlayerIndex(0));

	if (!ensure(LocalUserInfo && FirstUserInfo))
	{
		return;
	}

	// 检查硬性的平台/服务 ID
	RefreshLocalUserInfo(LocalUserInfo);

	FUniqueNetIdRepl FirstPlayerId = FirstUserInfo->GetNetId(EGameUserOnlineContext::PlatformOrDefault);

	// 检查登录失败后是否应创建游客。某些平台会返回成功,但会复用第一个玩家的 ID,这种情况计为失败
	if (LocalUserInfo != FirstUserInfo && LocalUserInfo->bCanBeGuest && (NewStatus == ELoginStatusType::NotLoggedIn || NetId == FirstPlayerId))
	{
		NetId = (FUniqueNetIdRef)FUniqueNetIdString::Create(FString::Printf(TEXT("GuestPlayer%d"), LocalUserInfo->LocalPlayerIndex), NULL_SUBSYSTEM);
		LocalUserInfo->bIsGuest = true;
		NewStatus = ELoginStatusType::UsingLocalProfile;
		Error.Reset();
		UE_LOG(LogGameUser, Log, TEXT("HandleLoginForUserInitialize created guest id %s for local player %d"), *NetId.ToString(), LocalUserInfo->LocalPlayerIndex);
	}
	else
	{
		LocalUserInfo->bIsGuest = false;
	}

	ensure(LocalUserInfo->IsDoingLogin());

	if (Error.IsSet())
	{
		FText ErrorText = GetErrorText(Error.GetValue());
		TimerManager.SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UGameUserSubsystem::HandleUserInitializeFailed, Params, ErrorText));
		return;
	}

	if (Context == EGameUserOnlineContext::Game)
	{
		LocalUserInfo->UpdateCachedNetId(NetId, EGameUserOnlineContext::Game);
	}

	ULocalPlayer* CurrentPlayer = GameInstance->GetLocalPlayerByIndex(LocalUserInfo->LocalPlayerIndex);
	if (!CurrentPlayer && Params.bCanCreateNewLocalPlayer)
	{
		FString ErrorString;
		CurrentPlayer = GameInstance->CreateLocalPlayer(LocalUserInfo->PlatformUser, ErrorString, true);

		if (!CurrentPlayer)
		{
			TimerManager.SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UGameUserSubsystem::HandleUserInitializeFailed, Params, FText::AsCultureInvariant(ErrorString)));
			return;
		}
		ensure(GameInstance->GetLocalPlayerByIndex(LocalUserInfo->LocalPlayerIndex) == CurrentPlayer);
	}

	// 如有需要,更新手柄和网络 ID
	SetLocalPlayerUserInfo(CurrentPlayer, LocalUserInfo);

	// 设置延迟回调
	TimerManager.SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UGameUserSubsystem::HandleUserInitializeSucceeded, Params));
}

void UGameUserSubsystem::HandleUserInitializeFailed(FGameUserInitializeParams Params, FText Error)
{
	UGameUserInfo* LocalUserInfo = ModifyInfo(GetUserInfoForLocalPlayerIndex(Params.LocalPlayerIndex));

	if (!LocalUserInfo)
	{
		// 自调度以来,用户信息已被重置
		return;
	}

	UE_LOG(LogGameUser, Warning, TEXT("TryToInitializeUser %d failed with error %s"), LocalUserInfo->LocalPlayerIndex, *Error.ToString());

	// 如果状态不正确,中止操作,因为我们可能已被取消
	if (!ensure(LocalUserInfo->IsDoingLogin()))
	{
		return;
	}

	// 如果初始登录失败,或我们最终完全登出,则设置为完全失败
	ELoginStatusType NewStatus = GetLocalUserLoginStatus(Params.PlatformUser, Params.OnlineContext);
	if (NewStatus == ELoginStatusType::NotLoggedIn || LocalUserInfo->InitializationState == EGameUserInitializationState::DoingInitialLogin)
	{
		LocalUserInfo->InitializationState = EGameUserInitializationState::FailedtoLogin;
	}
	else
	{
		LocalUserInfo->InitializationState = EGameUserInitializationState::LoggedInLocalOnly;
	}

	FText TitleText = NSLOCTEXT("GameUser", "LoginFailedTitle", "Login Failure");

	if (!Params.bSuppressLoginErrors)
	{
		SendSystemMessage(FGameUserTags::TAG_SystemMessage_Error_InitializeLocalPlayerFailed, TitleText, Error);
	}

	// 调用回调
	Params.OnUserInitializeComplete.ExecuteIfBound(LocalUserInfo, false, Error, Params.RequestedPrivilege, Params.OnlineContext);
	OnUserInitializeComplete.Broadcast(LocalUserInfo, false, Error, Params.RequestedPrivilege, Params.OnlineContext);
}

void UGameUserSubsystem::HandleUserInitializeSucceeded(FGameUserInitializeParams Params)
{
	UGameUserInfo* LocalUserInfo = ModifyInfo(GetUserInfoForLocalPlayerIndex(Params.LocalPlayerIndex));

	if (!LocalUserInfo)
	{
		// 自调度以来,用户信息已被重置
		return;
	}

	// 如果状态不正确,中止操作,因为我们可能已被取消
	if (!ensure(LocalUserInfo->IsDoingLogin()))
	{
		return;
	}

	// 修正状态
	if (Params.RequestedPrivilege == EGameUserPrivilege::CanPlayOnline)
	{
		LocalUserInfo->InitializationState = EGameUserInitializationState::LoggedInOnline;
	}
	else
	{
		LocalUserInfo->InitializationState = EGameUserInitializationState::LoggedInLocalOnly;
	}

	ensure(LocalUserInfo->GetPrivilegeAvailability(Params.RequestedPrivilege) == EGameUserAvailability::NowAvailable);

	// 调用回调
	Params.OnUserInitializeComplete.ExecuteIfBound(LocalUserInfo, true, FText(), Params.RequestedPrivilege, Params.OnlineContext);
	OnUserInitializeComplete.Broadcast(LocalUserInfo, true, FText(), Params.RequestedPrivilege, Params.OnlineContext);
}
