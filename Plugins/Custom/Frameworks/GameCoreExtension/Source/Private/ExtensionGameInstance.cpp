// Copyright © 2026 张鸿源. All Rights Reserved.


#include "ExtensionGameInstance.h"
#include "CommonUISettings.h"
#include "GameplayTagContainer.h"
#include "ICommonUIModule.h"
#include "Data/GameUserTypes.h"
#include "Engine/LocalPlayer.h"
#include "Logging/StructuredLog.h"
#include "MessagingSystem/DialogWidgetDescriptorBase.h"
#include "MessagingSystem/MessagingManager.h"
#include "Misc/GameCoreExtensionTags.h"
#include "Misc/LogGameCoreExtension.h"
#include "System/GameUserSessionSubsystem.h"
#include "System/GameUserSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExtensionGameInstance)

UExtensionGameInstance::UExtensionGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UExtensionGameInstance::Init()
{
	Super::Init();

	// 链接各个子系统
	FGameplayTagContainer PlatformTraits = ICommonUIModule::GetSettings().GetPlatformTraits();

	// 绑定链接用户系统
	UGameUserSubsystem* UserSubsystem = GetSubsystem<UGameUserSubsystem>();
	if (ensure(UserSubsystem))
	{
		UserSubsystem->SetTraitTags(PlatformTraits);
		UserSubsystem->OnHandleSystemMessage.AddDynamic(this, &ThisClass::HandleSystemMessage);
		UserSubsystem->OnUserPrivilegeChanged.AddDynamic(this, &ThisClass::HandlePrivilegeChanged);
		UserSubsystem->OnUserInitializeComplete.AddDynamic(this, &ThisClass::HandlerUserInitialized);
	}

	// 绑定链接会话系统
	UGameUserSessionSubsystem* SessionSubsystem = GetSubsystem<UGameUserSessionSubsystem>();
	if (ensure(SessionSubsystem))
	{
		SessionSubsystem->OnUserRequestedSessionEvent.AddUObject(this, &ThisClass::OnUserRequestedSession);
		SessionSubsystem->OnDestroySessionRequestedEvent.AddUObject(this, &ThisClass::OnDestroySessionRequested);
	}
}

void UExtensionGameInstance::ReturnToMainMenu()
{
	ResetUserAndSessionState();

	Super::ReturnToMainMenu();
}

void UExtensionGameInstance::HandleSystemMessage(FGameplayTag MessageType, FText Title, FText Message)
{
	ULocalPlayer* FirstPlayer = GetFirstGamePlayer();

	// 将严重错误转发到第一个玩家的错误对话框
	if (FirstPlayer && MessageType.MatchesTag(GameCoreExtensionTags::TAG_SystemMessage_Error))
	{
		if (UMessagingManager* Messaging = FirstPlayer->GetSubsystem<UMessagingManager>())
		{
			Messaging->ShowDialogInternal(GameCoreExtensionTags::TAG_SystemMessage_Error, UDialogWidgetDescriptorBase::CreateConfirmationOk(Title, Message), {});
		}
	}
}

void UExtensionGameInstance::HandlePrivilegeChanged(const UGameUserInfo* UserInfo, EGameUserPrivilege Privilege, EGameUserAvailability OldAvailability, EGameUserAvailability NewAvailability)
{
	// 默认情况下，如果第一个玩家的游戏权限丢失，则显示错误并断开连接
	if (Privilege == EGameUserPrivilege::CanPlay && OldAvailability == EGameUserAvailability::NowAvailable && NewAvailability != EGameUserAvailability::NowAvailable)
	{
		UE_LOG(LogGameCoreExtension, Error, TEXT("HandlePrivilegeChanged: Player %d no longer has permission to play the game!"), UserInfo->LocalPlayerIndex);
		// TODO: 游戏玩法可以在子类中做一些特定的事情
		// ReturnToMainMenu();
	}
}

void UExtensionGameInstance::HandlerUserInitialized(const UGameUserInfo* UserInfo, bool bSuccess, FText Error, EGameUserPrivilege RequestedPrivilege, EGameUserOnlineContext OnlineContext)
{
	// 子类可以覆盖这一点
}

void UExtensionGameInstance::ResetUserAndSessionState()
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

void UExtensionGameInstance::OnUserRequestedSession(const FPlatformUserId& PlatformUserId, UGameUserSession_SearchResult* InRequestedSession, const FOnlineResultInformation& RequestedSessionResult)
{
	if (InRequestedSession)
	{
		SetRequestedSession(InRequestedSession);
	}
	else
	{
		HandleSystemMessage(GameCoreExtensionTags::TAG_SystemMessage_Error, NSLOCTEXT("UExtensionGameInstance", "Warning_RequestedSessionFailed", "Requested Session Failed"), RequestedSessionResult.ErrorText);
	}
}

void UExtensionGameInstance::OnDestroySessionRequested(const FPlatformUserId& PlatformUserId, const FName& SessionName)
{
	// 当请求会话销毁时，请确保你的项目处于正确状态，可以销毁会话并转离

	UE_LOG(LogGameCoreExtension, Verbose, TEXT("[%hs] PlatformUserId:%d, SessionName: %s)"), __FUNCTION__, PlatformUserId.GetInternalId(), *SessionName.ToString());

	ReturnToMainMenu();
}

void UExtensionGameInstance::SetRequestedSession(UGameUserSession_SearchResult* InRequestedSession)
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

bool UExtensionGameInstance::CanJoinRequestedSession() const
{
	// 默认行为总是允许加入请求的会话
	return true;
}

void UExtensionGameInstance::JoinRequestedSession()
{
	if (RequestedSession)
	{
		if (ULocalPlayer* const FirstPlayer = GetFirstGamePlayer())
		{
			UGameUserSessionSubsystem* SessionSubsystem = GetSubsystem<UGameUserSessionSubsystem>();
			if (ensure(SessionSubsystem))
			{
				// 清理我们当前请求的会话，因为我们现在正在执行。
				UGameUserSession_SearchResult* TempRequestedSession = RequestedSession;
				RequestedSession = nullptr;
				SessionSubsystem->JoinSession(FirstPlayer->PlayerController, TempRequestedSession);
			}
		}
	}
}

void UExtensionGameInstance::ResetGameAndJoinRequestedSession()
{
	// 默认行为是返回主菜单。当游戏处于准备状态时，必须调用 JoinRequestedSession。
	ReturnToMainMenu();
}
