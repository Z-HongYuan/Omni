// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Infos/TeamPublicInfo.h"

#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TeamPublicInfo)

ATeamPublicInfo::ATeamPublicInfo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ATeamPublicInfo::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, TeamDisplayAsset, COND_InitialOnly);
}

void ATeamPublicInfo::OnRep_TeamDisplayAsset()
{
	TryRegisterWithTeamSubsystem();
}

void ATeamPublicInfo::SetTeamDisplayAsset(TObjectPtr<UTeamDisplayAssetBase> NewDisplayAsset)
{
	check(HasAuthority());
	check(TeamDisplayAsset == nullptr);

	TeamDisplayAsset = NewDisplayAsset;

	TryRegisterWithTeamSubsystem();
}
