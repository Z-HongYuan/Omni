// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Data/ExpUserFacingDefinition.h"

#include "Data/GameUserSession_HostSessionRequest.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "Misc/App.h"
#include "System/GameUserSessionSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExpUserFacingDefinition)

UGameUserSession_HostSessionRequest* UExpUserFacingDefinition::CreateHostingRequest(const UObject* WorldContextObject) const
{
	// 体验名会写进 URL，玩法名用于会话广告，两者都取主资产名
	const FString ExperienceName = ExperienceID.PrimaryAssetName.ToString();
	const FString PlaylistName = GetPrimaryAssetId().PrimaryAssetName.ToString();

	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;

	UGameUserSession_HostSessionRequest* Request = nullptr;
	if (UGameUserSessionSubsystem* SessionSubsystem = GameInstance ? GameInstance->GetSubsystem<UGameUserSessionSubsystem>() : nullptr)
	{
		Request = SessionSubsystem->CreateOnlineHostSessionRequest();
	}

	// 取不到会话子系统时自己造一个，保证离线环境下前端也能把请求构造出来
	if (Request == nullptr)
	{
		Request = NewObject<UGameUserSession_HostSessionRequest>(GetTransientPackage());
		Request->OnlineMode = EGameUserSessionOnlineMode::Online;
		Request->bUseLobbies = true;
		Request->bUseLobbiesVoiceChat = false;
		Request->bUsePresence = !IsRunningDedicatedServer();
	}

	Request->MapID = MapID;
	Request->ModeNameForAdvertisement = PlaylistName;
	Request->ExtraArgs = ExtraArgs;
	// 体验名最后写入，避免被 ExtraArgs 里的同名选项覆盖
	Request->ExtraArgs.Add(TEXT("Experience"), ExperienceName);
	Request->MaxPlayerCount = MaxPlayerCount;

	return Request;
}

TArray<UExpUserFacingDefinition*> UExpUserFacingDefinition::LoadAllPlaylists(bool bFrontEndOnly)
{
	TArray<UExpUserFacingDefinition*> Result;

	const FPrimaryAssetType PlaylistType(UExpUserFacingDefinition::StaticClass()->GetFName());
	UAssetManager& AssetManager = UAssetManager::Get();

	// 先同步加载，否则后面取到的只有已经加载过的对象
	TSharedPtr<FStreamableHandle> LoadHandle = AssetManager.LoadPrimaryAssetsWithType(PlaylistType);
	if (ensure(LoadHandle.IsValid()))
	{
		LoadHandle->WaitUntilComplete();
	}

	TArray<UObject*> LoadedObjects;
	AssetManager.GetPrimaryAssetObjectList(PlaylistType, LoadedObjects);
	Result.Reserve(LoadedObjects.Num());

	for (UObject* LoadedObject : LoadedObjects)
	{
		UExpUserFacingDefinition* Playlist = Cast<UExpUserFacingDefinition>(LoadedObject);
		if (Playlist == nullptr)
		{
			continue;
		}

		if (bFrontEndOnly && !Playlist->bShowInFrontEnd)
		{
			continue;
		}

		Result.Add(Playlist);
	}

	return Result;
}
