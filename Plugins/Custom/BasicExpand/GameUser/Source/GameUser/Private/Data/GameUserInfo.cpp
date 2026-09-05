// Copyright © 2026 张鸿源. All Rights Reserved.
// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Data/GameUserInfo.h"
#include "System/GameUserSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameUserInfo)

bool UGameUserInfo::IsLoggedIn() const
{
	return (InitializationState == EGameUserInitializationState::LoggedInLocalOnly || InitializationState == EGameUserInitializationState::LoggedInOnline);
}

bool UGameUserInfo::IsDoingLogin() const
{
	return (InitializationState == EGameUserInitializationState::DoingInitialLogin || InitializationState == EGameUserInitializationState::DoingNetworkLogin);
}

EGameUserPrivilegeResult UGameUserInfo::GetCachedPrivilegeResult(EGameUserPrivilege Privilege, EGameUserOnlineContext Context) const
{
	if (const FCachedData* FoundCached = GetCachedData(Context))
	{
		if (const EGameUserPrivilegeResult* FoundResult = FoundCached->CachedPrivileges.Find(Privilege))
		{
			return *FoundResult;
		}
	}
	return EGameUserPrivilegeResult::Unknown;
}

EGameUserAvailability UGameUserInfo::GetPrivilegeAvailability(EGameUserPrivilege Privilege) const
{
	// 无效的功能或用户
	if ((int32)Privilege < 0 || (int32)Privilege >= (int32)EGameUserPrivilege::Invalid_Count || InitializationState == EGameUserInitializationState::Invalid)
	{
		return EGameUserAvailability::Invalid;
	}

	EGameUserPrivilegeResult CachedResult = GetCachedPrivilegeResult(Privilege, EGameUserOnlineContext::Game);

	// 首先处理明确的失败情况
	switch (CachedResult)
	{
	case EGameUserPrivilegeResult::LicenseInvalid:
	case EGameUserPrivilegeResult::VersionOutdated:
	case EGameUserPrivilegeResult::AgeRestricted:
		return EGameUserAvailability::AlwaysUnavailable;

	case EGameUserPrivilegeResult::NetworkConnectionUnavailable:
	case EGameUserPrivilegeResult::AccountTypeRestricted:
	case EGameUserPrivilegeResult::AccountUseRestricted:
	case EGameUserPrivilegeResult::PlatformFailure:
		return EGameUserAvailability::CurrentlyUnavailable;

	default:
		break;
	}

	if (bIsGuest)
	{
		// 游客只能游玩,不能使用在线功能
		if (Privilege == EGameUserPrivilege::CanPlay)
		{
			return EGameUserAvailability::NowAvailable;
		}
		else
		{
			return EGameUserAvailability::AlwaysUnavailable;
		}
	}

	// 检查网络状态
	if (Privilege == EGameUserPrivilege::CanPlayOnline ||
		Privilege == EGameUserPrivilege::CanUseCrossPlay ||
		Privilege == EGameUserPrivilege::CanCommunicateViaTextOnline ||
		Privilege == EGameUserPrivilege::CanCommunicateViaVoiceOnline)
	{
		UGameUserSubsystem* Subsystem = GetSubsystem();
		if (ensure(Subsystem) && !Subsystem->HasOnlineConnection(EGameUserOnlineContext::Game))
		{
			return EGameUserAvailability::CurrentlyUnavailable;
		}
	}

	if (InitializationState == EGameUserInitializationState::FailedtoLogin)
	{
		// 之前的登录尝试失败
		return EGameUserAvailability::CurrentlyUnavailable;
	}
	else if (InitializationState == EGameUserInitializationState::Unknown || InitializationState == EGameUserInitializationState::DoingInitialLogin)
	{
		// 尚未登录
		return EGameUserAvailability::PossiblyAvailable;
	}
	else if (InitializationState == EGameUserInitializationState::LoggedInLocalOnly || InitializationState == EGameUserInitializationState::DoingNetworkLogin)
	{
		// 本地登录成功,因此游玩(CanPlay)检查有效
		if (Privilege == EGameUserPrivilege::CanPlay && CachedResult == EGameUserPrivilegeResult::Available)
		{
			return EGameUserAvailability::NowAvailable;
		}

		// 尚未在线登录
		return EGameUserAvailability::PossiblyAvailable;
	}
	else if (InitializationState == EGameUserInitializationState::LoggedInOnline)
	{
		// 已完全登录
		if (CachedResult == EGameUserPrivilegeResult::Available)
		{
			return EGameUserAvailability::NowAvailable;
		}

		// 因其他原因失败
		return EGameUserAvailability::CurrentlyUnavailable;
	}

	return EGameUserAvailability::Unknown;
}

FUniqueNetIdRepl UGameUserInfo::GetNetId(EGameUserOnlineContext Context) const
{
	if (const FCachedData* FoundCached = GetCachedData(Context))
	{
		return FoundCached->CachedNetId;
	}

	return FUniqueNetIdRepl();
}

FString UGameUserInfo::GetNickname(EGameUserOnlineContext Context) const
{
	if (const FCachedData* FoundCached = GetCachedData(Context))
	{
		return FoundCached->CachedNickname;
	}

	// TODO 这里或许应返回未知用户?
	return FString();
}

void UGameUserInfo::SetNickname(const FString& NewNickname, EGameUserOnlineContext Context)
{
	FCachedData* ContextCache = GetCachedData(Context);

	if (ensure(ContextCache))
	{
		ContextCache->CachedNickname = NewNickname;
	}
}

FString UGameUserInfo::GetDebugString() const
{
	FUniqueNetIdRepl NetId = GetNetId();
	return NetId.ToDebugString();
}

FPlatformUserId UGameUserInfo::GetPlatformUserId() const
{
	return PlatformUser;
}

int32 UGameUserInfo::GetPlatformUserIndex() const
{
	// 将我们的平台 ID 转换为索引
	const UGameUserSubsystem* Subsystem = GetSubsystem();

	if (ensure(Subsystem))
	{
		return Subsystem->GetPlatformUserIndexForId(PlatformUser);
	}

	return INDEX_NONE;
}

UGameUserInfo::FCachedData* UGameUserInfo::GetCachedData(EGameUserOnlineContext Context)
{
	// 直接查找,游戏(Game)上下文拥有独立于默认(Default)的缓存
	if (FCachedData* FoundData = CachedDataMap.Find(Context))
	{
		return FoundData;
	}

	// 现在尝试系统解析
	UGameUserSubsystem* Subsystem = GetSubsystem();

	EGameUserOnlineContext ResolvedContext = Subsystem->ResolveOnlineContext(Context);
	return CachedDataMap.Find(ResolvedContext);
}

const UGameUserInfo::FCachedData* UGameUserInfo::GetCachedData(EGameUserOnlineContext Context) const
{
	return const_cast<UGameUserInfo*>(this)->GetCachedData(Context);
}

void UGameUserInfo::UpdateCachedPrivilegeResult(EGameUserPrivilege Privilege, EGameUserPrivilegeResult Result, EGameUserOnlineContext Context)
{
	// 此函数只应在上下文已解析且类型有效时调用
	FCachedData* GameCache = GetCachedData(EGameUserOnlineContext::Game);
	FCachedData* ContextCache = GetCachedData(Context);

	if (!ensure(GameCache && ContextCache))
	{
		// 应该始终有效
		return;
	}

	// 先更新直接缓存
	ContextCache->CachedPrivileges.Add(Privilege, Result);

	if (GameCache != ContextCache)
	{
		// 查找另一个上下文以合并到游戏(Game)上下文
		EGameUserPrivilegeResult GameContextResult = Result;
		EGameUserPrivilegeResult OtherContextResult = EGameUserPrivilegeResult::Available;
		for (TPair<EGameUserOnlineContext, FCachedData>& Pair : CachedDataMap)
		{
			if (&Pair.Value != ContextCache && &Pair.Value != GameCache)
			{
				EGameUserPrivilegeResult* FoundResult = Pair.Value.CachedPrivileges.Find(Privilege);
				if (FoundResult)
				{
					OtherContextResult = *FoundResult;
				}
				else
				{
					OtherContextResult = EGameUserPrivilegeResult::Unknown;
				}
				break;
			}
		}

		if (GameContextResult == EGameUserPrivilegeResult::Available && OtherContextResult != EGameUserPrivilegeResult::Available)
		{
			// 另一个上下文的结果更差,采用该结果
			GameContextResult = OtherContextResult;
		}

		GameCache->CachedPrivileges.Add(Privilege, GameContextResult);
	}
}

void UGameUserInfo::UpdateCachedNetId(const FUniqueNetIdRepl& NewId, EGameUserOnlineContext Context)
{
	FCachedData* ContextCache = GetCachedData(Context);

	if (ensure(ContextCache))
	{
		ContextCache->CachedNetId = NewId;

		// 更新昵称
		const UGameUserSubsystem* Subsystem = GetSubsystem();
		if (ensure(Subsystem))
		{
			if (bIsGuest)
			{
				if (ContextCache->CachedNickname.IsEmpty())
				{
					// 如果昵称为空,则设置默认的游客名称,可通过 SetNickname 修改
					ContextCache->CachedNickname = NSLOCTEXT("GameUser", "GuestNickname", "Guest").ToString();
				}
			}
			else
			{
				// 使用系统昵称刷新,会覆盖 SetNickname 的设置
				ContextCache->CachedNickname = Subsystem->GetLocalUserNickname(GetPlatformUserId(), Context);
			}
		}
	}

	// 由于游客的工作方式,我们不合并 ID
}

UGameUserSubsystem* UGameUserInfo::GetSubsystem() const
{
	return Cast<UGameUserSubsystem>(GetOuter());
}
