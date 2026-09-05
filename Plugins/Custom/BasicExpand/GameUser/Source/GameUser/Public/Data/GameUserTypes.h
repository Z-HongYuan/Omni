// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

// 在线子系统(OSS v1)的头文件包含与前向声明
#include "OnlineSubsystemTypes.h"
#include "NativeGameplayTags.h"

#include "GameUserTypes.generated.h"

#define UE_API GAMEUSER_API

class IOnlineSubsystem;
struct FOnlineError;
using FOnlineErrorType = FOnlineError;
using ELoginStatusType = ELoginStatus::Type;

/** 枚举,指定在线查询的运行位置与方式 */
UENUM(BlueprintType)
enum class EGameUserOnlineContext : uint8
{
	/** 从游戏代码调用,使用默认系统,但带有特殊处理,可能合并来自多个上下文的结果 */
	Game,

	/** 引擎默认的在线系统,它始终存在,并且与 Service 或 Platform 之一相同 */
	Default,

	/** 显式请求外部服务,该服务可能不存在 */
	Service,

	/** 优先查找外部服务,找不到时回退到默认系统 */
	ServiceOrDefault,

	/** 显式请求平台系统,该系统可能不存在 */
	Platform,

	/** 优先查找平台系统,找不到时回退到默认系统 */
	PlatformOrDefault,

	/** 无效的系统 */
	Invalid
};

/** 枚举,描述特定用户的初始化状态 */
UENUM(BlueprintType)
enum class EGameUserInitializationState : uint8
{
	/** 用户尚未开始登录流程 */
	Unknown,

	/** 玩家正在通过本地登录获取用户 ID */
	DoingInitialLogin,

	/** 玩家正在执行网络登录,他们已在本地登录 */
	DoingNetworkLogin,

	/** 玩家完全未能登录 */
	FailedtoLogin,


	/** 玩家已登录并可以使用在线功能 */
	LoggedInOnline,

	/** 玩家已在本地登录(无论是访客还是真实用户),但无法执行在线操作 */
	LoggedInLocalOnly,


	/** 无效的状态或用户 */
	Invalid,
};

/** 枚举,指定用户可用的不同权限与能力 */
UENUM(BlueprintType)
enum class EGameUserPrivilege : uint8
{
	/** 用户是否可以在线或离线游玩 */
	CanPlay,

	/** 用户是否可以在线模式下游玩 */
	CanPlayOnline,

	/** 用户是否可以使用文字聊天 */
	CanCommunicateViaTextOnline,

	/** 用户是否可以使用语音聊天 */
	CanCommunicateViaVoiceOnline,

	/** 用户是否可以访问其他用户生成的内容 */
	CanUseUserGeneratedContent,

	/** 用户是否可以参与跨平台联机 */
	CanUseCrossPlay,

	/** 无效的权限(同时也是有效权限的计数) */
	Invalid_Count UMETA(Hidden)
};

/** 枚举,指定某项功能或权限的总体可用性,它综合了多个来源的信息 */
UENUM(BlueprintType)
enum class EGameUserAvailability : uint8
{
	/** 状态完全未知,需要查询 */
	Unknown,

	/** 该功能现在可以立即使用 */
	NowAvailable,

	/** 在正常登录流程完成后,该功能可能可用 */
	PossiblyAvailable,

	/** 该功能当前不可用(例如由于网络连接问题),但将来可能可用 */
	CurrentlyUnavailable,

	/** 由于账户或平台的硬性限制,该功能在本次会话剩余时间内将永远不可用 */
	AlwaysUnavailable,

	/** 无效的功能 */
	Invalid,
};

/** 枚举,给出用户可以使用或无法使用某项权限的具体原因 */
UENUM(BlueprintType)
enum class EGameUserPrivilegeResult : uint8
{
	/** 状态未知,需要查询 */
	Unknown,

	/** 该权限可以完全使用 */
	Available,

	/** 用户尚未完全登录 */
	UserNotLoggedIn,

	/** 用户不拥有该游戏或内容 */
	LicenseInvalid,

	/** 游戏需要更新或打补丁后该权限才可用 */
	VersionOutdated,

	/** 没有网络连接,重新连接可能会解决此问题 */
	NetworkConnectionUnavailable,

	/** 家长控制限制 */
	AgeRestricted,

	/** 账户没有所需的订阅或账户类型 */
	AccountTypeRestricted,

	/** 其他账户/用户限制,例如被服务封禁 */
	AccountUseRestricted,

	/** 其他平台特定的失败 */
	PlatformFailure,
};

/** 用于跟踪不同异步操作的进度 */
enum class EGameUserAsyncTaskState : uint8
{
	/** 任务尚未开始 */
	NotStarted,
	/** 任务正在处理中 */
	InProgress,
	/** 任务已成功完成 */
	Done,
	/** 任务未能完成 */
	Failed
};

/** 关于在线错误的详细信息。实际上是 FOnlineError 的包装器。 */
USTRUCT(BlueprintType)
struct FOnlineResultInformation
{
	GENERATED_BODY()

	/** 操作是否成功。如果成功,此结构体的错误字段将不包含额外信息。 */
	UPROPERTY(BlueprintReadOnly)
	bool bWasSuccessful = true;

	/** 唯一的错误 ID。可用于与特定已处理的错误进行比较。 */
	UPROPERTY(BlueprintReadOnly)
	FString ErrorId;

	/** 显示给用户的错误文本。 */
	UPROPERTY(BlueprintReadOnly)
	FText ErrorText;

	/**
	 * 从 FOnlineErrorType 初始化
	 * @param InOnlineError 要从中初始化的在线错误
	 */
	UE_API void FromOnlineError(const FOnlineErrorType& InOnlineError);
};

/** 通用用户子系统使用的标签列表 */
namespace FGameUserTags
{
	// 通用严重级别和特定的系统消息
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SystemMessage_Error)
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SystemMessage_Warning)
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SystemMessage_Display)

	/** 初始化玩家的所有尝试均失败,用户必须先执行某些操作才能再次尝试 */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_SystemMessage_Error_InitializeLocalPlayerFailed)

	// 平台特征标签,预期游戏实例或其他系统会针对相应的平台使用这些标签调用 SetTraitTags
	/** 该标签表示这是一个将控制器 ID 直接映射到不同系统用户的主机平台。如果为 false,同一个用户可以拥有多个控制器 */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Platform_Trait_RequiresStrictControllerMapping)

	/** 该标签表示平台只有一个在线用户,所有玩家都使用索引 0 */
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Platform_Trait_SingleOnlineUser)
};

#undef UE_API
