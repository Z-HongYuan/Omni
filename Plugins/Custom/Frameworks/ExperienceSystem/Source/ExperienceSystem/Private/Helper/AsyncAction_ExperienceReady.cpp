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
		// 世界无效时结束监听，不广播就绪。
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

		// 即使体验已就绪也延后一帧，避免调用方依赖同步完成。
		// TODO: 评估动态生成对象或加载界面结束后是否仍需延迟一帧。
		// TODO: 评估是否可统一使用体验加载阶段的随机延迟测试。
		// 下一帧检查操作仍有效后通知，并释放操作。
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
		// 等待体验回调后通知并释放操作，管理器在广播后清空委托列表。
		ExperienceComponent->CallOrRegister_OnExperienceLoaded(FExpLoaded::FDelegate::CreateLambda([WeakThis](const UExpDefinition* CurrentExperience)
		{
			WeakThis->OnReady.Broadcast();
			WeakThis->SetReadyToDestroy();
		}));
	}
}
