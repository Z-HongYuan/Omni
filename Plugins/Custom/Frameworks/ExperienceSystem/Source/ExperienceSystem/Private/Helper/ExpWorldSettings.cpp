// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Helper/ExpWorldSettings.h"

#include "EngineUtils.h"
#include "Engine/AssetManager.h"
#include "GameFramework/PlayerStart.h"
#include "Logs/LogExpSystem.h"
#include "Misc/UObjectToken.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExpWorldSettings)

AExpWorldSettings::AExpWorldSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

#if WITH_EDITOR
void AExpWorldSettings::CheckForErrors()
{
	Super::CheckForErrors();

	FMessageLog MapCheck("MapCheck");

	for (TActorIterator<APlayerStart> PlayerStartIt(GetWorld()); PlayerStartIt; ++PlayerStartIt)
	{
		APlayerStart* PlayerStart = *PlayerStartIt;
		if (IsValid(PlayerStart) && PlayerStart->GetClass() == APlayerStart::StaticClass())
		{
			MapCheck.Warning()
			        ->AddToken(FUObjectToken::Create(PlayerStart))
			        ->AddToken(FTextToken::Create(FText::FromString("is a normal APlayerStart, replace with AExpPlayerStart.")));
		}
	}

	// TODO: 在地图检查时验证软对象路径可解析为主资产 ID，避免指向未扫描的体验。
}
#endif

FPrimaryAssetId AExpWorldSettings::GetDefaultGameplayExperience() const
{
	FPrimaryAssetId Result;
	if (!DefaultGameplayExperience.IsNull())
	{
		Result = UAssetManager::Get().GetPrimaryAssetIdForPath(DefaultGameplayExperience.ToSoftObjectPath());

		if (!Result.IsValid())
		{
			UE_LOG(LogExpSystem, Error, TEXT("%s.DefaultGameplayExperience is %s but that failed to resolve into an asset ID (you might need to add a path to the Asset Rules in your game feature plugin or project settings"),
			       *GetPathNameSafe(this), *DefaultGameplayExperience.ToString());
		}
	}
	return Result;
}
