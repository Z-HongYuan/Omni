// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Infos/TeamInfoBase.h"

#include "Core/TeamSubsystem.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TeamInfoBase)

ATeamInfoBase::ATeamInfoBase(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
	, TeamId(INDEX_NONE)
{
	bReplicates = true;
	bAlwaysRelevant = true;
	NetPriority = 3.0f;
	SetReplicatingMovement(false);
}

void ATeamInfoBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, TeamId, COND_InitialOnly);
}

void ATeamInfoBase::BeginPlay()
{
	Super::BeginPlay();

	TryRegisterWithTeamSubsystem();
}

void ATeamInfoBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (TeamId != INDEX_NONE)
	{
		UTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UTeamSubsystem>();
		if (TeamSubsystem)
		{
			// EndPlay 期间子系统可能已销毁，仅在其仍有效时注销。
			TeamSubsystem->UnregisterTeamInfo(this);
		}
	}
	Super::EndPlay(EndPlayReason);
}

void ATeamInfoBase::RegisterWithTeamSubsystem(UTeamSubsystem* Subsystem)
{
	Subsystem->RegisterTeamInfo(this);
}

void ATeamInfoBase::TryRegisterWithTeamSubsystem()
{
	if (TeamId != INDEX_NONE)
	{
		UTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UTeamSubsystem>();
		if (ensure(TeamSubsystem))
		{
			RegisterWithTeamSubsystem(TeamSubsystem);
		}
	}
}

void ATeamInfoBase::SetTeamId(int32 NewTeamId)
{
	// TeamId 只在初始化时复制，因此只允许权威端首次赋值。
	check(HasAuthority());
	check(TeamId == INDEX_NONE);
	check(NewTeamId != INDEX_NONE);

	TeamId = NewTeamId;

	TryRegisterWithTeamSubsystem();
}

void ATeamInfoBase::OnRep_TeamId()
{
	TryRegisterWithTeamSubsystem();
}
