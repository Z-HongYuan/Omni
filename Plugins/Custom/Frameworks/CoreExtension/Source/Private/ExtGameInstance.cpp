// Copyright © 2026 张鸿源. All Rights Reserved.


#include "ExtGameInstance.h"
#include "CommonUISettings.h"
#include "GameplayTagContainer.h"
#include "ICommonUIModule.h"
#include "Data/GameUserTypes.h"
#include "Engine/LocalPlayer.h"
#include "Logging/StructuredLog.h"
#include "MessagingSystem/DialogWidgetDescriptorBase.h"
#include "MessagingSystem/MessagingManager.h"
#include "Misc/CoreExtensionTags.h"
#include "Misc/LogCoreExtension.h"
#include "System/GameUserSessionSubsystem.h"
#include "System/GameUserSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExtGameInstance)

UExtGameInstance::UExtGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UExtGameInstance::Init()
{
	Super::Init();

	// 读取平台特性，供用户系统判断当前平台能力。
	FGameplayTagContainer PlatformTraits = ICommonUIModule::GetSettings().GetPlatformTraits();

	// 向用户系统注册消息、权限和初始化回调。
	UGameUserSubsystem* UserSubsystem = GetSubsystem<UGameUserSubsystem>();
	if (ensure(UserSubsystem))
	{
		UserSubsystem->SetTraitTags(PlatformTraits);
		UserSubsystem->OnHandleSystemMessage.AddDynamic(this, &ThisClass::HandleSystemMessage);
		UserSubsystem->OnUserPrivilegeChanged.AddDynamic(this, &ThisClass::HandlePrivilegeChanged);
		UserSubsystem->OnUserInitializeComplete.AddDynamic(this, &ThisClass::HandlerUserInitialized);
	}

	// 接收外部会话邀请和会话销毁请求。
	UGameUserSessionSubsystem* SessionSubsystem = GetSubsystem<UGameUserSessionSubsystem>();
	if (ensure(SessionSubsystem))
	{
		SessionSubsystem->OnUserRequestedSessionEvent.AddUObject(this, &ThisClass::OnUserRequestedSession);
		SessionSubsystem->OnDestroySessionRequestedEvent.AddUObject(this, &ThisClass::OnDestroySessionRequested);
	}
}

void UExtGameInstance::ReturnToMainMenu()
{
	ResetUserAndSessionState();

	Super::ReturnToMainMenu();
}

void UExtGameInstance::HandleSystemMessage(FGameplayTag MessageType, FText Title, FText Message)
{
	ULocalPlayer* FirstPlayer = GetFirstGamePlayer();

	// 将错误转发到首个本地玩家的对话框。
	if (FirstPlayer && MessageType.MatchesTag(CoreExtensionTags::TAG_SystemMessage_Error))
	{
		if (UMessagingManager* Messaging = FirstPlayer->GetSubsystem<UMessagingManager>())
		{
			Messaging->ShowDialogInternal(CoreExtensionTags::TAG_SystemMessage_Error, UDialogWidgetDescriptorBase::CreateConfirmationOk(Title, Message), {});
		}
	}
}

void UExtGameInstance::HandlePrivilegeChanged(const UGameUserInfo* UserInfo, EGameUserPrivilege Privilege, EGameUserAvailability OldAvailability, EGameUserAvailability NewAvailability)
{
	// 玩家失去游戏权限时记录错误；当前实现不主动断开会话。
	if (Privilege == EGameUserPrivilege::CanPlay && OldAvailability == EGameUserAvailability::NowAvailable && NewAvailability != EGameUserAvailability::NowAvailable)
	{
		UE_LOG(LogCoreExtension, Error, TEXT("HandlePrivilegeChanged: Player %d no longer has permission to play the game!"), UserInfo->LocalPlayerIndex);
		// TODO: 在派生类中补充游戏权限丢失后的处理。
		// ReturnToMainMenu();
	}
}

void UExtGameInstance::HandlerUserInitialized(const UGameUserInfo* UserInfo, bool bSuccess, FText Error, EGameUserPrivilege RequestedPrivilege, EGameUserOnlineContext OnlineContext)
{
	// 留给派生类处理用户初始化结果。
}

void UExtGameInstance::ResetUserAndSessionState()
{
	UGameUserSubsystem* UserSubsystem = GetSubsystem<UGameUserSubsystem>();
	if (ensure(UserSubsystem))
	{
		UserSubsystem->ResetUserState();
	}

	UGameUserSessionSubsystem* SessionSubsystem = GetSubsystem<UGameUserSessionSubsystem>();
	if (ensure(SessionSubsystem))
	{
		SessionSubsystem->CleanUpSessions();
	}
}

void UExtGameInstance::OnUserRequestedSession(const FPlatformUserId& PlatformUserId, UGameUserSession_SearchResult* InRequestedSession, const FOnlineResultInformation& RequestedSessionResult)
{
	if (InRequestedSession)
	{
		SetRequestedSession(InRequestedSession);
	}
	else
	{
		HandleSystemMessage(CoreExtensionTags::TAG_SystemMessage_Error, NSLOCTEXT("UExtensionGameInstance", "Warning_RequestedSessionFailed", "Requested Session Failed"), RequestedSessionResult.ErrorText);
	}
}

void UExtGameInstance::OnDestroySessionRequested(const FPlatformUserId& PlatformUserId, const FName& SessionName)
{
	// 返回主菜单时统一清理用户和会话状态。

	UE_LOG(LogCoreExtension, Verbose, TEXT("[%hs] PlatformUserId:%d, SessionName: %s)"), __FUNCTION__, PlatformUserId.GetInternalId(), *SessionName.ToString());

	ReturnToMainMenu();
}

void UExtGameInstance::SetRequestedSession(UGameUserSession_SearchResult* InRequestedSession)
{
	RequestedSession = InRequestedSession;
	if (RequestedSession)
	{
		if (CanJoinRequestedSession())
		{
			JoinRequestedSession();
		}
		else
		{
			ResetGameAndJoinRequestedSession();
		}
	}
}

bool UExtGameInstance::CanJoinRequestedSession() const
{
	// 默认允许立即加入，派生类可根据游戏状态限制。
	return true;
}

void UExtGameInstance::JoinRequestedSession()
{
	if (RequestedSession)
	{
		if (ULocalPlayer* const FirstPlayer = GetFirstGamePlayer())
		{
			UGameUserSessionSubsystem* SessionSubsystem = GetSubsystem<UGameUserSessionSubsystem>();
			if (ensure(SessionSubsystem))
			{
				// 提交加入请求前清除缓存，避免后续流程重复消费。
				UGameUserSession_SearchResult* TempRequestedSession = RequestedSession;
				RequestedSession = nullptr;
				SessionSubsystem->JoinSession(FirstPlayer->PlayerController, TempRequestedSession);
			}
		}
	}
}

void UExtGameInstance::ResetGameAndJoinRequestedSession()
{
	// 默认行为是返回主菜单。当游戏处于准备状态时，必须调用 JoinRequestedSession。
	ReturnToMainMenu();
}
