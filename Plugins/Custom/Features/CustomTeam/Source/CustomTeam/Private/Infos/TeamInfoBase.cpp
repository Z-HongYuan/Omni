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
			// 终局游戏可能发生在子系统已经被摧毁的奇怪时间点
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
	// 仅能设置一次, 且必须在Authority端设置
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
