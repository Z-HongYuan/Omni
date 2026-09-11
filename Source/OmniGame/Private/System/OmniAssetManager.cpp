// Copyright © 2026 张鸿源. All Rights Reserved.


#include "System/OmniAssetManager.h"

#include "OmniGame/OmniGameLogChannel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniAssetManager)

UOmniAssetManager& UOmniAssetManager::Get()
{
	UOmniAssetManager* AssetManager = Cast<UOmniAssetManager>(&UAssetManager::Get());
	if (!AssetManager)
	{
		UE_LOG(LogOmniGame, Fatal, TEXT("AssetManagerClassName 配置错误，请在 DefaultEngine.ini 中设置为 /Script/OmniGame.OmniAssetManager。"));
	}

	return *AssetManager;
}

void UOmniAssetManager::StartInitialLoading()
{
	// 保留引擎的 Primary Asset 扫描和初始化流程。
	Super::StartInitialLoading();

	UE_LOG(LogOmniGame, Log, TEXT("OmniAssetManager StartInitialLoading: %s"), *GetClass()->GetPathName());
}
