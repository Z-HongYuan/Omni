// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Actions/AsyncAction_GameUserInitialize.h"

#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_GameUserInitialize)

UAsyncAction_GameUserInitialize* UAsyncAction_GameUserInitialize::InitializeForLocalPlay(UGameUserSubsystem* Target, int32 LocalPlayerIndex, FInputDeviceId PrimaryInputDevice, bool bCanUseGuestLogin)
{
	if (!PrimaryInputDevice.IsValid())
	{
		// 设置为默认设备
		PrimaryInputDevice = IPlatformInputDeviceMapper::Get().GetDefaultInputDevice();
	}

	UAsyncAction_GameUserInitialize* Action = NewObject<UAsyncAction_GameUserInitialize>();

	Action->RegisterWithGameInstance(Target);

	if (Target && Action->IsRegistered())
	{
		Action->Subsystem = Target;

		Action->Params.RequestedPrivilege = EGameUserPrivilege::CanPlay;
		Action->Params.LocalPlayerIndex = LocalPlayerIndex;
		Action->Params.PrimaryInputDevice = PrimaryInputDevice;
		Action->Params.bCanUseGuestLogin = bCanUseGuestLogin;
		Action->Params.bCanCreateNewLocalPlayer = true;
	}
	else
	{
		Action->SetReadyToDestroy();
	}

	return Action;
}

UAsyncAction_GameUserInitialize* UAsyncAction_GameUserInitialize::LoginForOnlinePlay(UGameUserSubsystem* Target, int32 LocalPlayerIndex)
{
	UAsyncAction_GameUserInitialize* Action = NewObject<UAsyncAction_GameUserInitialize>();

	Action->RegisterWithGameInstance(Target);

	if (Target && Action->IsRegistered())
	{
		Action->Subsystem = Target;

		Action->Params.RequestedPrivilege = EGameUserPrivilege::CanPlayOnline;
		Action->Params.LocalPlayerIndex = LocalPlayerIndex;
		Action->Params.bCanCreateNewLocalPlayer = false;
	}
	else
	{
		Action->SetReadyToDestroy();
	}

	return Action;
}

void UAsyncAction_GameUserInitialize::Activate()
{
	if (Subsystem.IsValid())
	{
		Params.OnUserInitializeComplete.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UAsyncAction_GameUserInitialize, HandleInitializationComplete));
		bool bSuccess = Subsystem->TryToInitializeUser(Params);

		if (!bSuccess)
		{
			// 在下一帧调用失败处理
			FTimerManager* TimerManager = GetTimerManager();

			if (TimerManager)
			{
				TimerManager->SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UAsyncAction_GameUserInitialize::HandleFailure));
			}
		}
	}
	else
	{
		SetReadyToDestroy();
	}
}

void UAsyncAction_GameUserInitialize::HandleFailure()
{
	const UGameUserInfo* UserInfo = nullptr;
	if (Subsystem.IsValid())
	{
		UserInfo = Subsystem->GetUserInfoForLocalPlayerIndex(Params.LocalPlayerIndex);
	}
	HandleInitializationComplete(UserInfo, false, NSLOCTEXT("GameUser", "LoginFailedEarly", "Unable to start login process"), Params.RequestedPrivilege, Params.OnlineContext);
}

void UAsyncAction_GameUserInitialize::HandleInitializationComplete(const UGameUserInfo* UserInfo, bool bSuccess, FText Error, EGameUserPrivilege RequestedPrivilege, EGameUserOnlineContext OnlineContext)
{
	if (ShouldBroadcastDelegates())
	{
		OnInitializationComplete.Broadcast(UserInfo, bSuccess, Error, RequestedPrivilege, OnlineContext);
	}

	SetReadyToDestroy();
}
