// Copyright © 2026 张鸿源. All Rights Reserved.
#include "Import/Materials/OmniImportToonProfile.h"
#include "AssetToolsModule.h"
#include "Engine/ToonProfile.h"
#include "Engine/SkeletalMesh.h"
#include "IAssetTools.h"
#include "MaterialEditingLibrary.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/PackageName.h"
#include "UObject/MetaData.h"
#include "UObject/Package.h"

namespace Omni::CharacterImport
{
namespace
{
const TCHAR* OwnedProfileKey = TEXT("Omni.CharacterImport.BodyToonProfile");
}

UToonProfile* ResolveBodyToonProfileTemplate(UMaterialInterface* Parent, UToonProfile* ExplicitTemplate)
{
	if (ExplicitTemplate) return ExplicitTemplate;
	if (!Parent) return nullptr;
	if (UToonProfile* Override = Parent->GetToonProfileOverride_Internal()) return Override;
	TSet<UToonProfile*> Profiles;
	for (uint32 Index = 0; Index < Parent->NumToonProfile_Internal(); ++Index)
	{
		if (UToonProfile* Profile = Parent->GetToonProfile_Internal(Index)) Profiles.Add(Profile);
	}
	return Profiles.Num() == 1 ? *Profiles.CreateConstIterator() : nullptr;
}

void SetOwnedToonProfile(UMaterialInstanceConstant* Material, UToonProfile* Profile)
{
	Material->Modify();
	Material->ToonProfileOverride = Profile;
	Material->bOverrideToonProfile = true;
	Material->GetOutermost()->GetMetaData().SetValue(Material, OwnedProfileKey, *Profile->GetPathName());
	UMaterialEditingLibrary::UpdateMaterialInstance(Material);
	(void)Material->MarkPackageDirty();
}

void ClearOwnedToonProfile(UMaterialInstanceConstant* Material)
{
	FMetaData& Metadata = Material->GetOutermost()->GetMetaData();
	const FString OwnedPath = Metadata.GetValue(Material, OwnedProfileKey);
	if (OwnedPath.IsEmpty()) return; // Old/unmarked assets may contain user assignments.
	Material->Modify();
	if (Material->ToonProfileOverride && Material->ToonProfileOverride->GetPathName() == OwnedPath)
	{
		Material->ToonProfileOverride = nullptr;
		Material->bOverrideToonProfile = false;
		UMaterialEditingLibrary::UpdateMaterialInstance(Material);
	}
	Metadata.RemoveValue(Material, OwnedProfileKey);
	(void)Material->MarkPackageDirty();
}

void ApplyBodyToonProfile(const TSet<UMaterialInstanceConstant*>& Targets, USkeletalMesh* Mesh,
	UToonProfile* Template, const FImportReporter& Report)
{
	if (Targets.IsEmpty() || !Mesh) return;
	// Sorting makes the destination deterministic even after materials are moved to different folders.
	TArray<UMaterialInstanceConstant*> Ordered = Targets.Array();
	Ordered.Sort([](const UMaterialInstanceConstant& A, const UMaterialInstanceConstant& B)
	{
		return A.GetPathName() < B.GetPathName();
	});
	const FString Folder = FPackageName::GetLongPackagePath(Ordered[0]->GetOutermost()->GetName());
	FString Name = Mesh->GetName();
	Name.RemoveFromStart(TEXT("SKM_"));
	Name = TEXT("TP_") + Name + TEXT("_Body");
	const FString Package = Folder / Name;
	const FString ObjectPath = Package + TEXT(".") + Name;
	UObject* Existing = StaticFindObject(UObject::StaticClass(), nullptr, *ObjectPath);
	if (!Existing && FPackageName::DoesPackageExist(Package)) Existing = LoadObject<UObject>(nullptr, *ObjectPath);
	UToonProfile* ModelProfile = Cast<UToonProfile>(Existing);
	if (!ModelProfile && (Existing || FPackageName::DoesPackageExist(Package)))
	{
		Report(TEXT("Body Toon Profile destination is occupied: ") + ObjectPath, true);
		return;
	}
	if (!ModelProfile)
	{
		if (!Template)
		{
			Report(TEXT("Body Toon Profile template is unavailable."), true);
			return;
		}
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();
		ModelProfile = Cast<UToonProfile>(AssetTools.DuplicateAsset(Name, Folder, Template));
	}
	if (!ModelProfile)
	{
		Report(TEXT("Could not create Body Toon Profile: ") + ObjectPath, true);
		return;
	}
	for (UMaterialInstanceConstant* Material : Ordered) SetOwnedToonProfile(Material, ModelProfile);
	Report(TEXT("Assigned model-local Toon Profile to Body only: ") + ModelProfile->GetPathName());
}
}
