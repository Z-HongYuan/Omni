// Copyright © 2026 张鸿源. All Rights Reserved.


#include "LoadingScreenTask.h"
#include "LoadingScreenManager.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LoadingScreenTask)

ULoadingScreenTask* ULoadingScreenTask::CreateLoadingScreenTask(UObject* WorldContextObject, const FString& ShowLoadingScreenReason)
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	ULoadingScreenManager* LoadingScreenManager = GameInstance ? GameInstance->GetSubsystem<ULoadingScreenManager>() : nullptr;

	//创建并注册加载处理器(查询器)到加载管理器中
	if (LoadingScreenManager)
	{
		ULoadingScreenTask* NewLoadingTask = NewObject<ULoadingScreenTask>(LoadingScreenManager);
		NewLoadingTask->SetShowLoadingScreenReason(ShowLoadingScreenReason);
		LoadingScreenManager->RegisterLoadingProcessor(NewLoadingTask);
		return NewLoadingTask;
	}

	return nullptr;
}

void ULoadingScreenTask::Unregister()
{
	//移除加载处理器(查询器), Outer 就是 LoadingScreenManager
	ULoadingScreenManager* LoadingScreenManager = Cast<ULoadingScreenManager>(GetOuter());
	LoadingScreenManager->UnregisterLoadingProcessor(this);
}

void ULoadingScreenTask::SetShowLoadingScreenReason(const FString& InReason)
{
	Reason = InReason;
}

bool ULoadingScreenTask::ShouldShowLoadingScreen(FString& OutReason) const
{
	OutReason = Reason;
	return true;
}
