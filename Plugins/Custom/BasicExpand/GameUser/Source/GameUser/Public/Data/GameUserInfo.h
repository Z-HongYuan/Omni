// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameUserTypes.h"

#include "GameUserInfo.generated.h"

#define UE_API GAMEUSER_API

class UGameUserSubsystem;

/** 单个用户的逻辑表示,每个已初始化的本地玩家都会有一个对应的此类对象 */
UCLASS(MinimalAPI, BlueprintType)
class UGameUserInfo : public UObject
{
	GENERATED_BODY()

public:
	/** 该用户的主要控制器输入设备,他们可能还拥有其他辅助设备 */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	FInputDeviceId PrimaryInputDevice;

	/** 指定本地平台上的逻辑用户,访客用户将指向主要用户 */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	FPlatformUserId PlatformUser;

	/** 如果此用户被分配了 LocalPlayer,一旦其完全创建,该值将与 GameInstance 的 localplayers 数组中的索引匹配 */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	int32 LocalPlayerIndex = -1;

	/** 如果为 true,则允许此用户作为访客 */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	bool bCanBeGuest = false;

	/** 如果为 true,则这是一个附加到主要用户 0 的访客用户 */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	bool bIsGuest = false;

	/** 用户初始化过程的整体状态 */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	EGameUserInitializationState InitializationState = EGameUserInitializationState::Invalid;

	/** 如果此用户已成功登录,则返回 true */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API bool IsLoggedIn() const;

	/** 如果此用户正在登录过程中,则返回 true */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API bool IsDoingLogin() const;

	/** 返回特定权限最近一次的查询结果,如果从未查询过则返回 unknown */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API EGameUserPrivilegeResult GetCachedPrivilegeResult(EGameUserPrivilege Privilege, EGameUserOnlineContext Context = EGameUserOnlineContext::Game) const;

	/** 查询某个功能的整体可用性,该结果结合了缓存结果与当前状态 */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API EGameUserAvailability GetPrivilegeAvailability(EGameUserPrivilege Privilege) const;

	/** 返回给定上下文对应的网络 ID(net id) */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API FUniqueNetIdRepl GetNetId(EGameUserOnlineContext Context = EGameUserOnlineContext::Game) const;

	/** 返回用户的可读昵称,将返回 UpdateCachedNetId 或 SetNickname 期间缓存的值 */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API FString GetNickname(EGameUserOnlineContext Context = EGameUserOnlineContext::Game) const;

	/** 修改用户的可读昵称,可用于设置多个访客,但对于真实用户,该值会被平台昵称覆盖 */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API void SetNickname(const FString& NewNickname, EGameUserOnlineContext Context = EGameUserOnlineContext::Game);

	/** 返回此玩家的内部调试字符串 */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API FString GetDebugString() const;

	/** 平台用户 ID 的访问器 */
	UE_API FPlatformUserId GetPlatformUserId() const;

	/** 获取平台用户索引,供需要整数的旧函数使用 */
	UE_API int32 GetPlatformUserIndex() const;

	// 内部数据,仅供在线子系统访问

	/** 每个在线系统的缓存数据 */
	struct FCachedData
	{
		/** 每个系统缓存的网络 ID */
		FUniqueNetIdRepl CachedNetId;

		/** 缓存的昵称,在网络 ID 可能发生变化时更新 */
		FString CachedNickname;

		/** 各种用户权限的缓存值 */
		TMap<EGameUserPrivilege, EGameUserPrivilegeResult> CachedPrivileges;
	};

	/** 每个上下文的缓存,game(游戏)上下文始终存在,但其他上下文可能不存在 */
	TMap<EGameUserOnlineContext, FCachedData> CachedDataMap;

	/** 使用解析规则查找缓存数据 */
	UE_API FCachedData* GetCachedData(EGameUserOnlineContext Context);
	UE_API const FCachedData* GetCachedData(EGameUserOnlineContext Context) const;

	/** 更新缓存的权限结果,必要时会传播到游戏 */
	UE_API void UpdateCachedPrivilegeResult(EGameUserPrivilege Privilege, EGameUserPrivilegeResult Result, EGameUserOnlineContext Context);

	/** 更新缓存的网络 ID,必要时会传播到游戏 */
	UE_API void UpdateCachedNetId(const FUniqueNetIdRepl& NewId, EGameUserOnlineContext Context);

	/** 返回拥有此对象的子系统 */
	UE_API UGameUserSubsystem* GetSubsystem() const;
};

#undef UE_API
