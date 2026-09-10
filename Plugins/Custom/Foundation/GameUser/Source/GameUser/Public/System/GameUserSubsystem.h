// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "OnlineError.h"
#include "Data/GameUserInfo.h"
#include "Data/GameUserTypes.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/WeakObjectPtr.h"

#include "GameUserSubsystem.generated.h"

#define UE_API GAMEUSER_API

/** 初始化过程成功或失败时触发的委托 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FGameUserOnInitializeCompleteMulticast, const UGameUserInfo*, UserInfo, bool, bSuccess, FText, Error, EGameUserPrivilege, RequestedPrivilege, EGameUserOnlineContext, OnlineContext);

DECLARE_DYNAMIC_DELEGATE_FiveParams(FGameUserOnInitializeComplete, const UGameUserInfo*, UserInfo, bool, bSuccess, FText, Error, EGameUserPrivilege, RequestedPrivilege, EGameUserOnlineContext, OnlineContext);

/** 发送系统错误消息时触发的委托,游戏可以根据类型标签选择是否向用户显示该消息 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FGameUserHandleSystemMessageDelegate, FGameplayTag, MessageType, FText, TitleText, FText, BodyText);

/** 权限发生变化时触发的委托,可绑定该委托以观察游戏过程中在线状态等是否发生变化 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FGameUserAvailabilityChangedDelegate, const UGameUserInfo*, UserInfo, EGameUserPrivilege, Privilege, EGameUserAvailability, OldAvailability, EGameUserAvailability, NewAvailability);

/** 初始化函数的参数结构体,通常由异步节点等包装函数填充 */
USTRUCT(BlueprintType)
struct FGameUserInitializeParams
{
	GENERATED_BODY()

	/** 要使用的本地玩家索引,如果允许创建玩家,则可以指定比当前更高的索引 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	int32 LocalPlayerIndex = 0;

	/** 已弃用的选择平台用户和输入设备的方法 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	int32 ControllerId = -1;

	/** 该用户的主要控制器输入设备,他们可能还拥有其他辅助设备 */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	FInputDeviceId PrimaryInputDevice;

	/** 指定本地平台上的逻辑用户 */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	FPlatformUserId PlatformUser;

	/** 通常为 CanPlay 或 CanPlayOnline,指定所需的权限级别 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	EGameUserPrivilege RequestedPrivilege = EGameUserPrivilege::CanPlay;

	/** 要登录的特定在线上下文,game 表示登录所有相关的上下文 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	EGameUserOnlineContext OnlineContext = EGameUserOnlineContext::Game;

	/** 如果允许为初始登录创建新的本地玩家,则为 true */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	bool bCanCreateNewLocalPlayer = false;

	/** 如果此玩家可以作为没有实际在线状态的访客用户,则为 true */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	bool bCanUseGuestLogin = false;

	/** 如果不应显示登录错误(由游戏负责显示),则为 true */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	bool bSuppressLoginErrors = false;

	/** 如果绑定了该动态委托,则在登录完成时调用它 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Default)
	FGameUserOnInitializeComplete OnUserInitializeComplete;
};

/**
 * 负责处理用户身份与登录状态的查询和变更的游戏子系统。
 * 每个游戏实例会创建一个子系统,可从蓝图或 C++ 代码访问。
 * 如果存在游戏特定的子类,则不会创建此基础子系统。
 */
UCLASS(MinimalAPI, BlueprintType, Config=Engine)
class UGameUserSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;
	UE_API virtual bool ShouldCreateSubsystem(UObject* Outer) const override;


	/** 当任何请求的初始化请求完成时调用的蓝图委托 */
	UPROPERTY(BlueprintAssignable, Category = GameUser)
	FGameUserOnInitializeCompleteMulticast OnUserInitializeComplete;

	/** 当系统发送错误/警告消息时调用的蓝图委托 */
	UPROPERTY(BlueprintAssignable, Category = GameUser)
	FGameUserHandleSystemMessageDelegate OnHandleSystemMessage;

	/** 当用户的权限可用性发生变化时调用的蓝图委托  */
	UPROPERTY(BlueprintAssignable, Category = GameUser)
	FGameUserAvailabilityChangedDelegate OnUserPrivilegeChanged;

	/** 通过 OnHandleSystemMessage 发送系统消息 */
	UFUNCTION(BlueprintCallable, Category = GameUser)
	UE_API virtual void SendSystemMessage(FGameplayTag MessageType, FText TitleText, FText BodyText);

	/** 设置本地玩家的最大数量,不会销毁现有的玩家 */
	UFUNCTION(BlueprintCallable, Category = GameUser)
	UE_API virtual void SetMaxLocalPlayers(int32 InMaxLocalPLayers);

	/** 获取本地玩家的最大数量 */
	UFUNCTION(BlueprintPure, Category = GameUser)
	UE_API int32 GetMaxLocalPlayers() const;

	/** 获取当前本地玩家的数量,始终至少为 1 */
	UFUNCTION(BlueprintPure, Category = GameUser)
	UE_API int32 GetNumLocalPlayers() const;

	/** 返回指定本地玩家的初始化状态 */
	UFUNCTION(BlueprintPure, Category = GameUser)
	UE_API EGameUserInitializationState GetLocalPlayerInitializationState(int32 LocalPlayerIndex) const;

	/** 返回游戏实例中给定本地玩家索引对应的用户信息,在运行中的游戏里索引 0 始终有效 */
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = GameUser)
	UE_API const UGameUserInfo* GetUserInfoForLocalPlayerIndex(int32 LocalPlayerIndex) const;

	/** 已弃用,请在有可用时改用 PlatformUserId */
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = GameUser)
	UE_API const UGameUserInfo* GetUserInfoForPlatformUserIndex(int32 PlatformUserIndex) const;

	/** 返回给定平台用户索引对应的主要用户信息。可能返回 null */
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = GameUser)
	UE_API const UGameUserInfo* GetUserInfoForPlatformUser(FPlatformUserId PlatformUser) const;

	/** 返回唯一网络 ID 对应的用户信息。可能返回 null */
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = GameUser)
	UE_API const UGameUserInfo* GetUserInfoForUniqueNetId(const FUniqueNetIdRepl& NetId) const;

	/** 已弃用,请在有可用时改用 InputDeviceId */
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = GameUser)
	UE_API const UGameUserInfo* GetUserInfoForControllerId(int32 ControllerId) const;

	/** 返回给定输入设备对应的用户信息。可能返回 null */
	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = GameUser)
	UE_API const UGameUserInfo* GetUserInfoForInputDevice(FInputDeviceId InputDevice) const;

	/**
	 * 尝试启动创建或更新本地玩家的流程,包括登录和创建玩家控制器。
	 * 当流程成功或失败时,将广播 OnUserInitializeComplete 委托。
	 *
	 * @param LocalPlayerIndex	Game Instance 中 LocalPlayer 的期望索引,0 为主要玩家,1 及以上用于本地多人
	 * @param PrimaryInputDevice 应映射到此用户的物理控制器,若无效则使用默认设备
	 * @param bCanUseGuestLogin	如果为 true,此玩家可以是没有真实 Unique Net Id 的访客
	 *
	 * @returns 如果流程已启动则返回 true,如果在正常启动前失败则返回 false
	 */
	UFUNCTION(BlueprintCallable, Category = GameUser)
	UE_API virtual bool TryToInitializeForLocalPlay(int32 LocalPlayerIndex, FInputDeviceId PrimaryInputDevice, bool bCanUseGuestLogin);

	/**
	 * 启动将本地已登录用户进行完整在线登录的流程,包括账户权限检查。
	 * 当流程成功或失败时,将广播 OnUserInitializeComplete 委托。
	 *
	 * @param LocalPlayerIndex	Game Instance 中现有 LocalPlayer 的索引
	 *
	 * @returns 如果流程已启动则返回 true,如果在正常启动前失败则返回 false
	 */
	UFUNCTION(BlueprintCallable, Category = GameUser)
	UE_API virtual bool TryToLoginForOnlinePlay(int32 LocalPlayerIndex);

	/**
	 * 启动常规的用户登录和初始化流程,使用参数结构体来决定登录目标。
	 * 当流程成功或失败时,将广播 OnUserInitializeComplete 委托。
	 * AsyncAction_GameUserInitialize 提供了多个包装函数,便于在事件图表中使用该功能。
	 *
	 * @returns 如果流程已启动则返回 true,如果在正常启动前失败则返回 false
	 */
	UFUNCTION(BlueprintCallable, Category = GameUser)
	UE_API virtual bool TryToInitializeUser(FGameUserInitializeParams Params);

	/** 
	 * 启动监听新控制器和现有控制器的用户输入并将其登录的流程。
	 * 这会在活动的 GameViewportClient 上插入一个按键输入处理器,再次以空按键数组调用即可关闭。
	 *
	 * @param AnyUserKeys		监听这些按键以匹配任意用户(包括默认用户)。用于初始的按任意键开始画面,设为空以禁用
	 * @param NewUserKeys		监听这些按键以匹配没有玩家控制器的新用户。用于分屏/本地多人,设为空以禁用
	 * @param Params			检测到按键输入后传递给 TryToInitializeUser 的参数
	 */
	UFUNCTION(BlueprintCallable, Category = GameUser)
	UE_API virtual void ListenForLoginKeyInput(TArray<FKey> AnyUserKeys, TArray<FKey> NewUserKeys, FGameUserInitializeParams Params);

	/** 尝试取消进行中的初始化,这在所有平台上可能无效,但会禁用回调 */
	UFUNCTION(BlueprintCallable, Category = GameUser)
	UE_API virtual bool CancelUserInitialization(int32 LocalPlayerIndex);

	/** 将玩家从所有在线系统中登出,并且可选地在不是第一个玩家时将其完全销毁 */
	UFUNCTION(BlueprintCallable, Category = GameUser)
	UE_API virtual bool TryToLogOutUser(int32 LocalPlayerIndex, bool bDestroyPlayer = false);

	/** 出错后返回主菜单时,重置登录和初始化状态 */
	UFUNCTION(BlueprintCallable, Category = GameUser)
	UE_API virtual void ResetUserState();

	/** 如果这是一个拥有有效身份的潜在真实平台用户(即使当前未登录),则返回 true  */
	UE_API virtual bool IsRealPlatformUserIndex(int32 PlatformUserIndex) const;

	/** 如果这是一个拥有有效身份的潜在真实平台用户(即使当前未登录),则返回 true */
	UE_API virtual bool IsRealPlatformUser(FPlatformUserId PlatformUser) const;

	/** 将索引转换为 ID */
	UE_API virtual FPlatformUserId GetPlatformUserIdForIndex(int32 PlatformUserIndex) const;

	/** 将 ID 转换为索引 */
	UE_API virtual int32 GetPlatformUserIndexForId(FPlatformUserId PlatformUser) const;

	/** 获取输入设备对应的用户 */
	UE_API virtual FPlatformUserId GetPlatformUserIdForInputDevice(FInputDeviceId InputDevice) const;

	/** 获取用户的主要输入设备 ID */
	UE_API virtual FInputDeviceId GetPrimaryInputDeviceForPlatformUser(FPlatformUserId PlatformUser) const;

	/** 当平台状态或选项发生变化时,由游戏代码调用以设置缓存的特性标签 */
	UE_API virtual void SetTraitTags(const FGameplayTagContainer& InTags);

	/** 获取当前影响功能可用性的标签 */
	const FGameplayTagContainer& GetTraitTags() const { return CachedTraitTags; }

	/** 检查特定的平台/功能标签是否已启用 */
	UFUNCTION(BlueprintPure, Category=GameUser)
	bool HasTraitTag(const FGameplayTag TraitTag) const { return CachedTraitTags.HasTag(TraitTag); }

	/** 检查启动时是否应显示按开始键/输入确认画面。游戏可以调用此函数,或直接检查特性标签 */
	UFUNCTION(BlueprintPure, BlueprintPure, Category=GameUser)
	UE_API virtual bool ShouldWaitForStartInput() const;


	// 用于访问底层在线系统信息的函数

	/** 返回特定类型的 OSS 接口,如果没有该类型则返回 null */
	UE_API IOnlineSubsystem* GetOnlineSubsystem(EGameUserOnlineContext Context = EGameUserOnlineContext::Game) const;

	/** 返回特定类型的身份接口,如果没有该类型则返回 null */
	UE_API IOnlineIdentity* GetOnlineIdentity(EGameUserOnlineContext Context = EGameUserOnlineContext::Game) const;

	/** 返回 OSS 系统的可读名称 */
	UE_API FName GetOnlineSubsystemName(EGameUserOnlineContext Context = EGameUserOnlineContext::Game) const;

	/** 返回当前的在线连接状态 */
	UE_API EOnlineServerConnectionStatus::Type GetConnectionStatus(EGameUserOnlineContext Context = EGameUserOnlineContext::Game) const;

	/** 如果当前已连接到后端服务器,则返回 true */
	UE_API bool HasOnlineConnection(EGameUserOnlineContext Context = EGameUserOnlineContext::Game) const;

	/** 返回玩家在指定在线系统上的当前登录状态,仅对真实平台用户有效 */
	UE_API ELoginStatusType GetLocalUserLoginStatus(FPlatformUserId PlatformUser, EGameUserOnlineContext Context = EGameUserOnlineContext::Game) const;

	/** 返回本地平台用户的唯一网络 ID */
	UE_API FUniqueNetIdRepl GetLocalUserNetId(FPlatformUserId PlatformUser, EGameUserOnlineContext Context = EGameUserOnlineContext::Game) const;

	/** 返回本地平台用户的昵称,该值缓存在通用用户信息中 */
	UE_API FString GetLocalUserNickname(FPlatformUserId PlatformUser, EGameUserOnlineContext Context = EGameUserOnlineContext::Game) const;

	/** 将用户 ID 转换为调试字符串 */
	UE_API FString PlatformUserIdToString(FPlatformUserId UserId);

	/** 将上下文转换为调试字符串 */
	UE_API FString EGameUserOnlineContextToString(EGameUserOnlineContext Context);

	/** 返回权限检查的可读字符串 */
	UE_API virtual FText GetPrivilegeDescription(EGameUserPrivilege Privilege) const;
	UE_API virtual FText GetPrivilegeResultDescription(EGameUserPrivilegeResult Result) const;

	/** 
	 * 为现有本地用户启动登录流程,如果回调未安排则返回 false 
	 * 这会激活底层状态机,但不会修改用户信息上的初始化状态
	 */
	DECLARE_DELEGATE_FiveParams(FOnLocalUserLoginCompleteDelegate, const UGameUserInfo* /*UserInfo*/, ELoginStatusType /*NewStatus*/, FUniqueNetIdRepl /*NetId*/, const TOptional<FOnlineErrorType>& /*Error*/,
	                            EGameUserOnlineContext /*Type*/);
	UE_API virtual bool LoginLocalUser(const UGameUserInfo* UserInfo, EGameUserPrivilege RequestedPrivilege, EGameUserOnlineContext Context, FOnLocalUserLoginCompleteDelegate OnComplete);

	/** 将本地玩家分配给特定的本地用户,并按需调用回调 */
	UE_API virtual void SetLocalPlayerUserInfo(ULocalPlayer* LocalPlayer, const UGameUserInfo* UserInfo);

	/** 将具有默认行为的上下文解析为特定上下文 */
	UE_API EGameUserOnlineContext ResolveOnlineContext(EGameUserOnlineContext Context) const;

	/** 如果存在独立的平台和服务接口,则为 true */
	UE_API bool HasSeparatePlatformContext() const;

protected:
	/** 缓存每个在线上下文的状态和指针的内部结构体 */
	struct FOnlineContextCache
	{
		/** 指向基础子系统的指针,只要游戏实例存在就一直有效 */
		IOnlineSubsystem* OnlineSubsystem = nullptr;

		/** 缓存的身份系统,它将始终有效 */
		IOnlineIdentityPtr IdentityInterface;

		/** 最后传入 HandleNetworkConnectionStatusChanged 处理器的连接状态 */
		EOnlineServerConnectionStatus::Type CurrentConnectionStatus = EOnlineServerConnectionStatus::Normal;

		/** 重置状态,清除所有共享指针非常重要 */
		void Reset()
		{
			OnlineSubsystem = nullptr;
			IdentityInterface.Reset();
			CurrentConnectionStatus = EOnlineServerConnectionStatus::Normal;
		}
	};

	/** 表示进行中的登录请求的内部结构体 */
	struct FUserLoginRequest : public TSharedFromThis<FUserLoginRequest>
	{
		FUserLoginRequest(UGameUserInfo* InUserInfo, EGameUserPrivilege InPrivilege, EGameUserOnlineContext InContext, FOnLocalUserLoginCompleteDelegate&& InDelegate)
			: UserInfo(TWeakObjectPtr<UGameUserInfo>(InUserInfo))
			  , DesiredPrivilege(InPrivilege)
			  , DesiredContext(InContext)
			  , Delegate(MoveTemp(InDelegate))
		{
		}

		/** 正在尝试登录的本地用户 */
		TWeakObjectPtr<UGameUserInfo> UserInfo;

		/** 登录请求的整体状态,可能来自多个来源 */
		EGameUserAsyncTaskState OverallLoginState = EGameUserAsyncTaskState::NotStarted;

		/** 尝试使用平台认证的状态。对于 OSSv1,一旦启动,该状态会立即转为 Failed,因为我们在 OSSv1 中不支持平台认证。 */
		EGameUserAsyncTaskState TransferPlatformAuthState = EGameUserAsyncTaskState::NotStarted;

		/** 尝试使用 AutoLogin 的状态 */
		EGameUserAsyncTaskState AutoLoginState = EGameUserAsyncTaskState::NotStarted;

		/** 尝试使用外部登录 UI 的状态 */
		EGameUserAsyncTaskState LoginUIState = EGameUserAsyncTaskState::NotStarted;

		/** 最终请求的权限 */
		EGameUserPrivilege DesiredPrivilege = EGameUserPrivilege::Invalid_Count;

		/** 尝试请求相关权限的状态 */
		EGameUserAsyncTaskState PrivilegeCheckState = EGameUserAsyncTaskState::NotStarted;

		/** 最终登录的上下文 */
		EGameUserOnlineContext DesiredContext = EGameUserOnlineContext::Invalid;

		/** 当前正在登录的在线系统 */
		EGameUserOnlineContext CurrentContext = EGameUserOnlineContext::Invalid;

		/** 完成时的用户回调 */
		FOnLocalUserLoginCompleteDelegate Delegate;

		/** 要显示给用户的最新/相关错误 */
		TOptional<FOnlineErrorType> Error;
	};


	/** 创建新的用户信息对象 */
	UE_API virtual UGameUserInfo* CreateLocalUserInfo(int32 LocalPlayerIndex);

	/** 用于 const 获取器的去 const 包装函数 */
	FORCEINLINE UGameUserInfo* ModifyInfo(const UGameUserInfo* Info) { return const_cast<UGameUserInfo*>(Info); }

	/** 从 OSS 刷新用户信息 */
	UE_API virtual void RefreshLocalUserInfo(UGameUserInfo* UserInfo);

	/** 可能发送权限可用性通知,将当前值与缓存的旧值进行比较 */
	UE_API virtual void HandleChangedAvailability(UGameUserInfo* UserInfo, EGameUserPrivilege Privilege, EGameUserAvailability OldAvailability);

	/** 更新用户上缓存的权限并通知委托 */
	UE_API virtual void UpdateUserPrivilegeResult(UGameUserInfo* UserInfo, EGameUserPrivilege Privilege, EGameUserPrivilegeResult Result, EGameUserOnlineContext Context);

	/** 获取某种在线系统的内部数据,对于 service 可能返回 null */
	UE_API const FOnlineContextCache* GetContextCache(EGameUserOnlineContext Context = EGameUserOnlineContext::Game) const;
	UE_API FOnlineContextCache* GetContextCache(EGameUserOnlineContext Context = EGameUserOnlineContext::Game);

	/** 在绑定委托之前创建并设置系统对象 */
	UE_API virtual void CreateOnlineContexts();
	UE_API virtual void DestroyOnlineContexts();

	/** 绑定在线委托 */
	UE_API virtual void BindOnlineDelegates();

	/** 强制登出并反初始化单个用户 */
	UE_API virtual void LogOutLocalUser(FPlatformUserId PlatformUser);

	/** 执行登录请求的下一步(可能包含完成该请求)。如果完成则返回 true */
	UE_API virtual void ProcessLoginRequest(TSharedRef<FUserLoginRequest> Request);

	/** 在 OSS 上调用登录,使用来自平台 OSS 的平台认证。如果 AutoLogin 已启动则返回 true */
	UE_API virtual bool TransferPlatformAuth(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);

	/** 在 OSS 上调用 AutoLogin。如果 AutoLogin 已启动则返回 true。 */
	UE_API virtual bool AutoLogin(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);

	/** 在 OSS 上调用 ShowLoginUI。如果 ShowLoginUI 已启动则返回 true。 */
	UE_API virtual bool ShowLoginUI(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);

	/** 在 OSS 上调用 QueryUserPrivilege。如果 QueryUserPrivilege 已启动则返回 true。 */
	UE_API virtual bool QueryUserPrivilege(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);

	/** OSS 特定函数 */
	UE_API virtual EGameUserPrivilege ConvertOSSPrivilege(EUserPrivileges::Type Privilege) const;
	UE_API virtual EUserPrivileges::Type ConvertOSSPrivilege(EGameUserPrivilege Privilege) const;
	UE_API virtual EGameUserPrivilegeResult ConvertOSSPrivilegeResult(EUserPrivileges::Type Privilege, uint32 Results) const;

	UE_API void BindOnlineDelegatesOSSv1();
	UE_API bool AutoLoginOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
	UE_API bool ShowLoginUIOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);
	UE_API bool QueryUserPrivilegeOSSv1(FOnlineContextCache* System, TSharedRef<FUserLoginRequest> Request, FPlatformUserId PlatformUser);

	/** OSS 函数的回调 */
	UE_API virtual void HandleIdentityLoginStatusChanged(int32 PlatformUserIndex, ELoginStatus::Type OldStatus, ELoginStatus::Type NewStatus, const FUniqueNetId& NewId, EGameUserOnlineContext Context);
	UE_API virtual void HandleUserLoginCompleted(int32 PlatformUserIndex, bool bWasSuccessful, const FUniqueNetId& NetId, const FString& Error, EGameUserOnlineContext Context);
	UE_API virtual void HandleControllerPairingChanged(int32 PlatformUserIndex, FControllerPairingChangedUserInfo PreviousUser, FControllerPairingChangedUserInfo NewUser);
	UE_API virtual void HandleNetworkConnectionStatusChanged(const FString& ServiceName, EOnlineServerConnectionStatus::Type LastConnectionStatus, EOnlineServerConnectionStatus::Type ConnectionStatus, EGameUserOnlineContext Context);
	UE_API virtual void HandleOnLoginUIClosed(TSharedPtr<const FUniqueNetId> LoggedInNetId, const int PlatformUserIndex, const FOnlineError& Error, EGameUserOnlineContext Context);
	UE_API virtual void HandleCheckPrivilegesComplete(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults, EGameUserPrivilege RequestedPrivilege, TWeakObjectPtr<UGameUserInfo> GameUserInfo,
	                                                  EGameUserOnlineContext Context);

	/**
	 * 当输入设备(例如游戏手柄)连接或断开时触发的回调。 
	 */
	UE_API virtual void HandleInputDeviceConnectionChanged(EInputDeviceConnectionState NewConnectionState, FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId);

	UE_API virtual void HandleLoginForUserInitialize(const UGameUserInfo* UserInfo, ELoginStatusType NewStatus, FUniqueNetIdRepl NetId, const TOptional<FOnlineErrorType>& Error, EGameUserOnlineContext Context,
	                                                 FGameUserInitializeParams Params);
	UE_API virtual void HandleUserInitializeFailed(FGameUserInitializeParams Params, FText Error);
	UE_API virtual void HandleUserInitializeSucceeded(FGameUserInitializeParams Params);

	/** 处理按开始键/登录逻辑的回调 */
	UE_API virtual bool OverrideInputKeyForLogin(FInputKeyEventArgs& EventArgs);


	/** 之前的覆盖处理器,取消时将恢复 */
	FOverrideInputKeyHandler WrappedInputKeyHandler;

	/** 监听任意用户按键的按键列表 */
	TArray<FKey> LoginKeysForAnyUser;

	/** 监听新未映射用户按键的按键列表 */
	TArray<FKey> LoginKeysForNewUser;

	/** 按键触发的登录所使用的参数 */
	FGameUserInitializeParams ParamsForLoginKey;

	/** 本地玩家的最大数量 */
	int32 MaxNumberOfLocalPlayers = 0;

	/** 如果这是不需要 LocalPlayer 的专用服务器,则为 true */
	bool bIsDedicatedServer = false;

	/** 当前进行中的登录请求列表 */
	TArray<TSharedRef<FUserLoginRequest>> ActiveLoginRequests;

	/** 每个本地用户的信息,从本地玩家索引到用户 */
	UPROPERTY()
	TMap<int32, TObjectPtr<UGameUserInfo>> LocalUserInfos;

	/** 缓存的平台/模式特性标签 */
	FGameplayTagContainer CachedTraitTags;

	/** 除初始化期间外,请勿访问此成员 */
	FOnlineContextCache* DefaultContextInternal = nullptr;
	FOnlineContextCache* ServiceContextInternal = nullptr;
	FOnlineContextCache* PlatformContextInternal = nullptr;

	friend UGameUserInfo;
};

#undef UE_API
