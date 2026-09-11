// Copyright Epic Games, Inc. All Rights Reserved.

#include "PocketLevel/PocketLevelInstance.h"

#include "Engine/Level.h"
#include "Engine/LevelStreaming.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "PocketLevel/PocketLevel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PocketLevelInstance)

bool UPocketLevelInstance::Initialize(ULocalPlayer* InLocalPlayer, UPocketLevel* InPocketLevel, FVector InSpawnPoint)
{
	if (!IsValid(InLocalPlayer) || !IsValid(InLocalPlayer->GetWorld()) || !IsValid(InPocketLevel) || InPocketLevel->SubLevel.IsNull())
	{
		return false;
	}

	if (!ensure(StreamingPocketLevel == nullptr))
	{
		return false;
	}

	LocalPlayer = InLocalPlayer;
	World = LocalPlayer->GetWorld();
	PocketLevel = InPocketLevel;
	Bounds = FBoxSphereBounds(FSphere(InSpawnPoint, PocketLevel->BoundsSize.GetAbsMax()));

	bool bSuccess = false;
	StreamingPocketLevel = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(LocalPlayer, PocketLevel->SubLevel, Bounds.Origin, FRotator::ZeroRotator, bSuccess);
	if (!bSuccess || !IsValid(StreamingPocketLevel))
	{
		return false;
	}

	StreamingPocketLevel->OnLevelLoaded.AddUniqueDynamic(this, &ThisClass::HandlePocketLevelLoaded);
	StreamingPocketLevel->OnLevelShown.AddUniqueDynamic(this, &ThisClass::HandlePocketLevelShown);
	return true;
}

void UPocketLevelInstance::StreamIn()
{
	if (StreamingPocketLevel)
	{
		StreamingPocketLevel->SetShouldBeVisible(true);
		StreamingPocketLevel->SetShouldBeLoaded(true);
	}
}

void UPocketLevelInstance::StreamOut()
{
	if (StreamingPocketLevel)
	{
		StreamingPocketLevel->SetShouldBeVisible(false);
		StreamingPocketLevel->SetShouldBeLoaded(false);
	}
}

FDelegateHandle UPocketLevelInstance::AddReadyCallback(FPocketLevelInstanceEvent::FDelegate Callback)
{
	if (!IsValid(StreamingPocketLevel))
	{
		return FDelegateHandle();
	}

	if (StreamingPocketLevel->GetLevelStreamingState() == ELevelStreamingState::LoadedVisible)
	{
		Callback.ExecuteIfBound(this);
	}

	return OnReadyEvent.Add(Callback);
}

void UPocketLevelInstance::RemoveReadyCallback(FDelegateHandle CallbackToRemove)
{
	OnReadyEvent.Remove(CallbackToRemove);
}

void UPocketLevelInstance::BeginDestroy()
{
	Super::BeginDestroy();

	if (StreamingPocketLevel)
	{
		StreamingPocketLevel->bShouldBlockOnUnload = false;
		StreamingPocketLevel->SetShouldBeLoaded(false);
		StreamingPocketLevel->OnLevelShown.RemoveAll(this);
		StreamingPocketLevel->OnLevelLoaded.RemoveAll(this);
		StreamingPocketLevel = nullptr;
	}
}

void UPocketLevelInstance::HandlePocketLevelLoaded()
{
	if (StreamingPocketLevel)
	{
		// 让关卡里的一切都按“客户端本地生成”的方式建立，
		// 而不是走 bExchangedRoles = true 那条路径 —— 后者虽然同样是在客户端生成，
		// 但含义是“服务器要求这么做的”，并且服务器稍后还会
		// 把这些 Actor 的信息同步过来。
		if (ULevel* LoadedLevel = StreamingPocketLevel->GetLoadedLevel())
		{
			LoadedLevel->bClientOnlyVisible = true;

			for (AActor* Actor : LoadedLevel->Actors)
			{
				if (Actor)
				{
					Actor->bExchangedRoles = true; // 临时黑科技：等 bClientOnlyVisible 足以满足全部需求后即可移除。
				}
			}

			// TODO: 共享的口袋空间不应该被设置归属。
			if (LocalPlayer)
			{
				if (APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld()))
				{
					for (AActor* Actor : LoadedLevel->Actors)
					{
						if (Actor)
						{
							Actor->SetOwner(PC);
						}
					}
				}
			}
		}
	}
}

void UPocketLevelInstance::HandlePocketLevelShown()
{
	OnReadyEvent.Broadcast(this);
}
