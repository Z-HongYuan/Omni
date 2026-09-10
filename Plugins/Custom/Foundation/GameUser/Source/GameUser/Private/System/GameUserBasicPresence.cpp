// Copyright © 2026 张鸿源. All Rights Reserved.

#include "System/GameUserBasicPresence.h"

#include "LogGameUser.h"
#include "OnlineSubsystemUtils.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "System/GameUserSessionSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameUserBasicPresence)

void UGameUserBasicPresence::Initialize(FSubsystemCollectionBase& Collection)
{
	UGameUserSessionSubsystem* GameSession = Collection.InitializeDependency<UGameUserSessionSubsystem>();
	if (ensure(GameSession))
	{
		GameSession->OnSessionInformationChangedEvent.AddUObject(this, &UGameUserBasicPresence::OnNotifySessionInformationChanged);
	}
}

void UGameUserBasicPresence::Deinitialize()
{
	Super::Deinitialize();
}

void UGameUserBasicPresence::OnNotifySessionInformationChanged(EGameUserSessionInformationState SessionStatus, const FString& GameMode, const FString& MapName)
{
	if (bEnableSessionsBasedPresence && !GetGameInstance()->IsDedicatedServerInstance())
	{
		// 由于地图名是一个 URL,将其修剪
		FString MapNameTruncated = MapName;
		if (!MapNameTruncated.IsEmpty())
		{
			int LastIndexOfSlash = 0;
			MapNameTruncated.FindLastChar('/', LastIndexOfSlash);
			MapNameTruncated = MapNameTruncated.RightChop(LastIndexOfSlash + 1);
		}

		if (IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld()))
		{
			IOnlinePresencePtr Presence = OnlineSub->GetPresenceInterface();
			if (Presence)
			{
				FOnlineUserPresenceStatus UpdatedPresence;
				UpdatedPresence.State = EOnlinePresenceState::Online; // 仅当用户拥有有效的 UniqueNetId 时我们才会发送在线状态更新,因此可以假定他们处于在线状态
				UpdatedPresence.StatusStr = *SessionStateToBackendKey(SessionStatus);
				UpdatedPresence.Properties.Emplace(PresenceKeyGameMode, GameMode);
				UpdatedPresence.Properties.Emplace(PresenceKeyMapName, MapNameTruncated);

				for (const ULocalPlayer* LocalPlayer : GetGameInstance()->GetLocalPlayers())
				{
					if (LocalPlayer && LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId() != nullptr)
					{
						Presence->SetPresence(*LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), UpdatedPresence);
					}
				}
			}
		}
	}
}

FString UGameUserBasicPresence::SessionStateToBackendKey(EGameUserSessionInformationState SessionStatus)
{
	switch (SessionStatus)
	{
	case EGameUserSessionInformationState::OutOfGame:
		return PresenceStatusMainMenu;
		break;
	case EGameUserSessionInformationState::Matchmaking:
		return PresenceStatusMatchmaking;
		break;
	case EGameUserSessionInformationState::InGame:
		return PresenceStatusInGame;
		break;
	default:
		UE_LOG(LogUserBasicPresence, Error, TEXT("UGameUserBasicPresence::SessionStateToBackendKey: Found unknown enum value %d"), (uint8)SessionStatus);
		return TEXT("Unknown");
		break;
	}
}
