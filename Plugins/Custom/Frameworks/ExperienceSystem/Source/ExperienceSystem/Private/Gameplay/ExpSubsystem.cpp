// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Gameplay/ExpSubsystem.h"

#include "Components/GameFrameworkComponentManager.h"
#include "Data/ExpSystemTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExpSubsystem)

void UExpSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// 声明需要依赖于 GameFrameworkComponentManager 进行初始化
	Collection.InitializeDependency<UGameFrameworkComponentManager>();
	Super::Initialize(Collection);

	InitComponentStateChain();
}

void UExpSubsystem::InitComponentStateChain()
{
	// 初始化组件状态链
	UGameInstance* GI = GetGameInstance();
	UGameFrameworkComponentManager* ComponentManager = GI->GetSubsystem<UGameFrameworkComponentManager>();

	if (ensure(ComponentManager))
	{
		ComponentManager->RegisterInitState(ExpSystemTags::TAG_InitState_Spawned, false, FGameplayTag());
		ComponentManager->RegisterInitState(ExpSystemTags::TAG_InitState_DataAvailable, false, ExpSystemTags::TAG_InitState_Spawned);
		ComponentManager->RegisterInitState(ExpSystemTags::TAG_InitState_DataInitialized, false, ExpSystemTags::TAG_InitState_DataAvailable);
		ComponentManager->RegisterInitState(ExpSystemTags::TAG_InitState_GameplayReady, false, ExpSystemTags::TAG_InitState_DataInitialized);
	}
}
