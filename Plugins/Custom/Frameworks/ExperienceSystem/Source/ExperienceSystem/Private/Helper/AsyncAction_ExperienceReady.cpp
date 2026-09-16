// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Helper/AsyncAction_ExperienceReady.h"

#include "Components/ExpManagerComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Logs/LogExpSystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_ExperienceReady)

UAsyncAction_ExperienceReady* UAsyncAction_ExperienceReady::WaitForExperienceReady(UObject* WorldContextObject)
{
	UAsyncAction_ExperienceReady* Action = nullptr;

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		Action = NewObject<UAsyncAction_ExperienceReady>();
		Action->RegisterWithGameInstance(World);
	}

	return Action;
}

void UAsyncAction_ExperienceReady::Activate()
{
	if (UWorld* World = RegisteredWithGameInstance->GetWorld())
	{
		if (AGameStateBase* GameState = World->GetGameState())
		{
			ListenToExperienceLoading(GameState);
		}
		else
		{
			World->GameStateSetEvent.AddWeakLambda(this, [this](AGameStateBase* NewState)
			{
				ListenToExperienceLoading(NewState);
			});
		}
	}
	else
	{
		// 如果无效的话,直接结束监听
		UE_LOG(LogExpSystem, Warning, TEXT("UAsyncAction_ExperienceReady::Activate: World is null"));
		SetReadyToDestroy();
	}
}

void UAsyncAction_ExperienceReady::SetReadyToDestroy()
{
	if (UWorld* World = RegisteredWithGameInstance->GetWorld())
	{
		World->GameStateSetEvent.RemoveAll(this);
	}
	Super::SetReadyToDestroy();
}

void UAsyncAction_ExperienceReady::ListenToExperienceLoading(const AGameStateBase* GameState)
{
	check(GameState);
	UExpManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UExpManagerComponent>();
	check(ExperienceComponent);

	TWeakObjectPtr<UAsyncAction_ExperienceReady> WeakThis(this);

	if (ExperienceComponent->IsExperienceLoaded())
	{
		UWorld* World = GameState->GetWorld();
		check(World);

		// The experience happened to be already loaded, but still delay a frame to
		// make sure people don't write stuff that relies on this always being true
		//@TODO: Consider not delaying for dynamically spawned stuff / any time after the loading screen has dropped?
		//@TODO: Maybe just inject a random 0-1s delay in the experience load itself?
		// 已加载的情况下,下帧触发委托并且销毁异步操作
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([WeakThis]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->OnReady.Broadcast();
				WeakThis->SetReadyToDestroy();
			}
		}));
	}
	else
	{
		// 如果还在加载,就注册一个委托,当体验完成后触发,体验加载完成后会自动销毁委托
		ExperienceComponent->CallOrRegister_OnExperienceLoaded(FExpLoaded::FDelegate::CreateLambda([WeakThis](const UExpDefinition* CurrentExperience)
		{
			WeakThis->OnReady.Broadcast();
			WeakThis->SetReadyToDestroy();
		}));
	}
}
