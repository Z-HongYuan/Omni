// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Gameplay/ExperienceSubsystem.h"

#include "Components/GameFrameworkComponentManager.h"
#include "Data/ExperienceSystemTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExperienceSubsystem)

void UExperienceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// 声明需要依赖于 GameFrameworkComponentManager 进行初始化
	Collection.InitializeDependency<UGameFrameworkComponentManager>();
	Super::Initialize(Collection);

	InitComponentStateChain();
}

void UExperienceSubsystem::InitComponentStateChain()
{
	// 初始化组件状态链
	UGameInstance* GI = GetGameInstance();
	UGameFrameworkComponentManager* ComponentManager = GI->GetSubsystem<UGameFrameworkComponentManager>();

	if (ensure(ComponentManager))
	{
		ComponentManager->RegisterInitState(ExperienceSystemTags::TAG_InitState_Spawned, false, FGameplayTag());
		ComponentManager->RegisterInitState(ExperienceSystemTags::TAG_InitState_DataAvailable, false, ExperienceSystemTags::TAG_InitState_Spawned);
		ComponentManager->RegisterInitState(ExperienceSystemTags::TAG_InitState_DataInitialized, false, ExperienceSystemTags::TAG_InitState_DataAvailable);
		ComponentManager->RegisterInitState(ExperienceSystemTags::TAG_InitState_GameplayReady, false, ExperienceSystemTags::TAG_InitState_DataInitialized);
	}
}
