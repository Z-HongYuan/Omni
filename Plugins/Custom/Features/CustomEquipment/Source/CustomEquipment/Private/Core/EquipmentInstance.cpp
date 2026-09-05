// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Core/EquipmentInstance.h"

#include "Components/SkeletalMeshComponent.h"
#include "Core/EquipmentDefinition.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EquipmentInstance)

UEquipmentInstance::UEquipmentInstance()
{
}

void UEquipmentInstance::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Instigator);
	DOREPLIFETIME(ThisClass, SpawnedActors);
}

void UEquipmentInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	using namespace UE::Net;

	// 构建描述符并为该对象分配PropertyReplicationFragments
	FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}

UWorld* UEquipmentInstance::GetWorld() const
{
	if (APawn* OwningPawn = GetPawn())
	{
		return OwningPawn->GetWorld();
	}
	else
	{
		return nullptr;
	}
}

APawn* UEquipmentInstance::GetPawn() const
{
	return Cast<APawn>(GetOuter());
}

APawn* UEquipmentInstance::GetTypedPawn(TSubclassOf<APawn> PawnType) const
{
	APawn* Result = nullptr;
	if (UClass* ActualPawnType = PawnType)
	{
		if (GetOuter()->IsA(ActualPawnType))
		{
			Result = Cast<APawn>(GetOuter());
		}
	}
	return Result;
}

void UEquipmentInstance::SpawnEquipmentActors(const TArray<FEquipmentActorToSpawn>& ActorsToSpawn)
{
	if (APawn* OwningPawn = GetPawn())
	{
		USceneComponent* AttachTarget = OwningPawn->GetRootComponent();
		if (ACharacter* Char = Cast<ACharacter>(OwningPawn))
		{
			AttachTarget = Char->GetMesh();
		}

		for (const FEquipmentActorToSpawn& SpawnInfo : ActorsToSpawn)
		{
			AActor* NewActor = GetWorld()->SpawnActorDeferred<AActor>(SpawnInfo.ActorToSpawn, FTransform::Identity, OwningPawn);
			NewActor->FinishSpawning(FTransform::Identity, true);
			NewActor->SetActorRelativeTransform(SpawnInfo.AttachTransform);
			NewActor->AttachToComponent(AttachTarget, FAttachmentTransformRules::KeepRelativeTransform, SpawnInfo.AttachSocket);

			SpawnedActors.Add(NewActor);
		}
	}
}

void UEquipmentInstance::DestroyEquipmentActors()
{
	for (AActor* Actor : SpawnedActors)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}
}

void UEquipmentInstance::OnEquipped()
{
	K2_OnEquipped();
}

void UEquipmentInstance::OnUnequipped()
{
	K2_OnUnequipped();
}

void UEquipmentInstance::OnRep_Instigator()
{
}
