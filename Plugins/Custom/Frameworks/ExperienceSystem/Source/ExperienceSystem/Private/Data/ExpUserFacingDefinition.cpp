// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Data/ExpUserFacingDefinition.h"

#include "Data/GameUserSession_HostSessionRequest.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "Misc/App.h"
#include "Misc/PackageName.h"
#include "System/GameUserSessionSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExpUserFacingDefinition)

bool UExpUserFacingDefinition::CanHostExperience() const
{
	const UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
	if (!AssetManager || !MapID.IsValid() || !ExperienceID.IsValid())
	{
		return false;
	}

	const FSoftObjectPath MapPath = AssetManager->GetPrimaryAssetPath(MapID);
	const FSoftObjectPath ExperiencePath = AssetManager->GetPrimaryAssetPath(ExperienceID);
	return MapPath.IsValid()
		&& ExperiencePath.IsValid()
		&& FPackageName::DoesPackageExist(MapPath.GetLongPackageName())
		&& FPackageName::DoesPackageExist(ExperiencePath.GetLongPackageName());
}

UGameUserSession_HostSessionRequest* UExpUserFacingDefinition::CreateHostingRequest(const UObject* WorldContextObject) const
{
	// 体验名称写入 URL，玩法定义名称用于会话广告。
	const FString ExperienceName = ExperienceID.PrimaryAssetName.ToString();
	const FString PlaylistName = GetPrimaryAssetId().PrimaryAssetName.ToString();

	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;

	UGameUserSession_HostSessionRequest* Request = nullptr;
	if (UGameUserSessionSubsystem* SessionSubsystem = GameInstance ? GameInstance->GetSubsystem<UGameUserSessionSubsystem>() : nullptr)
	{
		Request = SessionSubsystem->CreateOnlineHostSessionRequest();
	}

	// 会话子系统不可用时仍构造请求；这里只创建请求对象，不代表已建立会话。
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
	// 最后写入 Experience，确保额外参数不能覆盖所选体验。
	Request->ExtraArgs.Add(TEXT("Experience"), ExperienceName);
	Request->MaxPlayerCount = MaxPlayerCount;

	return Request;
}

TArray<UExpUserFacingDefinition*> UExpUserFacingDefinition::LoadAllPlaylists(bool bFrontEndOnly)
{
	TArray<UExpUserFacingDefinition*> Result;

	const FPrimaryAssetType PlaylistType(UExpUserFacingDefinition::StaticClass()->GetFName());
	UAssetManager& AssetManager = UAssetManager::Get();

	// 等待主资产加载完成，避免列表只包含此前已驻留的对象。
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
