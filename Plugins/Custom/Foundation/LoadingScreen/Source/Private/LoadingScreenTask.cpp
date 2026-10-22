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

	// 任务注册到管理器后参与显示条件查询；调用方仍需持有任务。
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
	// 通过创建时的 Outer 找回管理器，撤销本任务的加载请求。
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
