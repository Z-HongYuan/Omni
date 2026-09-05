// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Core/CosmeticCheats.h"

#include "Core/CosmeticsControllerComponent.h"
#include "Core/CosmeticHelper.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CosmeticCheats)

UCosmeticsControllerComponent* UCosmeticCheats::GetCosmeticComponent() const
{
	if (APlayerController* PC = GetPlayerController())
	{
		return PC->FindComponentByClass<UCosmeticsControllerComponent>();
	}
	return nullptr;
}


UCosmeticCheats::UCosmeticCheats()
{
	/*
	 * 向作弊管理器添加拓展
	 */
#if UE_WITH_CHEAT_MANAGER
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		UCheatManager::RegisterForOnCheatManagerCreated(FOnCheatManagerCreated::FDelegate::CreateLambda(
			[](UCheatManager* CheatManager)
			{
				CheatManager->AddCheatManagerExtension(NewObject<ThisClass>(CheatManager));
			}));
	}
#endif
}

void UCosmeticCheats::AddCharacterPart(const FString& AssetName, bool bSuppressNaturalParts)
{
#if UE_WITH_CHEAT_MANAGER
	if (UCosmeticsControllerComponent* CosmeticComponent = GetCosmeticComponent())
	{
		TSubclassOf<AActor> PartClass = UCosmeticHelper::FindClassByShortName<AActor>(AssetName);
		if (PartClass != nullptr)
		{
			FCharacterPart Part;
			Part.PartClass = PartClass;

			CosmeticComponent->AddCheatPart(Part, bSuppressNaturalParts);
		}
	}
#endif
}

void UCosmeticCheats::ReplaceCharacterPart(const FString& AssetName, bool bSuppressNaturalParts)
{
	ClearCharacterPartOverrides();
	AddCharacterPart(AssetName, bSuppressNaturalParts);
}

void UCosmeticCheats::ClearCharacterPartOverrides()
{
#if UE_WITH_CHEAT_MANAGER
	if (UCosmeticsControllerComponent* CosmeticComponent = GetCosmeticComponent())
	{
		CosmeticComponent->ClearCheatParts();
	}
#endif
}
