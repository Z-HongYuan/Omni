// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Gameplay/ExpSubsystem.h"

#include "Components/GameFrameworkComponentManager.h"
#include "Data/ExpSystemTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExpSubsystem)

void UExpSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// 先初始化 GameFrameworkComponentManager，再注册状态链。
	Collection.InitializeDependency<UGameFrameworkComponentManager>();
	Super::Initialize(Collection);

	InitComponentStateChain();
}

void UExpSubsystem::InitComponentStateChain()
{
	// 在当前 GameInstance 的组件管理器中注册状态顺序。
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
