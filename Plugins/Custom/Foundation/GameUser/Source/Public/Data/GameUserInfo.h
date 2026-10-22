// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameUserTypes.h"

#include "GameUserInfo.generated.h"

#define UE_API GAMEUSER_API

class UGameUserSubsystem;

/** 保存本地用户的初始化状态、设备映射，以及各在线上下文的身份和权限缓存。 */
UCLASS(MinimalAPI, BlueprintType)
class UGameUserInfo : public UObject
{
	GENERATED_BODY()

public:
	/** 用户的主要输入设备；同一用户还可能关联其他设备。 */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	FInputDeviceId PrimaryInputDevice;

	/** 关联的逻辑平台用户；访客身份另由 bIsGuest 和缓存网络 ID 表示。 */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	FPlatformUserId PlatformUser;

	/** 对应 GameInstance 本地玩家数组中的索引，创建 LocalPlayer 时保持一致。 */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	int32 LocalPlayerIndex = -1;

	/** 如果为 true，则允许此用户作为访客。 */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	bool bCanBeGuest = false;

	/** 如果为 true，则这是一个附加到主要用户 0 的访客用户。 */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	bool bIsGuest = false;

	/** 用户初始化过程的整体状态。 */
	UPROPERTY(BlueprintReadOnly, Category = UserInfo)
	EGameUserInitializationState InitializationState = EGameUserInitializationState::Invalid;

	/** 如果此用户已成功登录，则返回 true。 */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API bool IsLoggedIn() const;

	/** 如果此用户正在登录过程中，则返回 true。 */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API bool IsDoingLogin() const;

	/** 返回指定上下文的权限缓存；未查询或没有缓存时返回 Unknown。 */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API EGameUserPrivilegeResult GetCachedPrivilegeResult(EGameUserPrivilege Privilege, EGameUserOnlineContext Context = EGameUserOnlineContext::Game) const;

	/** 查询某个功能的整体可用性，该结果结合了缓存结果与当前状态。 */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API EGameUserAvailability GetPrivilegeAvailability(EGameUserPrivilege Privilege) const;

	/** 返回给定上下文对应的网络 ID(net id)。 */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API FUniqueNetIdRepl GetNetId(EGameUserOnlineContext Context = EGameUserOnlineContext::Game) const;

	/** 返回用户的可读昵称，将返回 UpdateCachedNetId 或 SetNickname 期间缓存的值。 */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API FString GetNickname(EGameUserOnlineContext Context = EGameUserOnlineContext::Game) const;

	/** 修改用户的可读昵称，可用于设置多个访客，但对于真实用户，该值会被平台昵称覆盖。 */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API void SetNickname(const FString& NewNickname, EGameUserOnlineContext Context = EGameUserOnlineContext::Game);

	/** 返回此玩家的内部调试字符串。 */
	UFUNCTION(BlueprintCallable, Category = UserInfo)
	UE_API FString GetDebugString() const;

	/** 平台用户 ID 的访问器。 */
	UE_API FPlatformUserId GetPlatformUserId() const;

	/** 获取平台用户索引，供需要整数的旧函数使用。 */
	UE_API int32 GetPlatformUserIndex() const;

	// 在线上下文缓存与内部更新接口。

	/** 每个在线系统的缓存数据。 */
	struct FCachedData
	{
		/** 每个系统缓存的网络 ID。 */
		FUniqueNetIdRepl CachedNetId;

		/** 缓存的昵称，在网络 ID 可能发生变化时更新。 */
		FString CachedNickname;

		/** 各种用户权限的缓存值。 */
		TMap<EGameUserPrivilege, EGameUserPrivilegeResult> CachedPrivileges;
	};

	/** 按在线上下文保存的缓存；Game 和 Default 在用户信息创建时初始化。 */
	TMap<EGameUserOnlineContext, FCachedData> CachedDataMap;

	/** 使用解析规则查找缓存数据。 */
	UE_API FCachedData* GetCachedData(EGameUserOnlineContext Context);
	UE_API const FCachedData* GetCachedData(EGameUserOnlineContext Context) const;

	/** 更新指定上下文的权限缓存，并将其他上下文的限制合并到 Game 缓存。 */
	UE_API void UpdateCachedPrivilegeResult(EGameUserPrivilege Privilege, EGameUserPrivilegeResult Result, EGameUserOnlineContext Context);

	/** 更新指定上下文的网络 ID 和昵称；不自动合并到其他上下文。 */
	UE_API void UpdateCachedNetId(const FUniqueNetIdRepl& NewId, EGameUserOnlineContext Context);

	/** 返回拥有此对象的子系统。 */
	UE_API UGameUserSubsystem* GetSubsystem() const;
};

#undef UE_API
