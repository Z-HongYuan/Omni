// Copyright © 2026 张鸿源. All Rights Reserved.
#include "Import/Materials/OmniImportToonProfile.h"
#include "Import/Materials/OmniImportMaterialUserData.h"
#include "AssetToolsModule.h"
#include "Engine/ToonProfile.h"
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

UToonProfile* FindOwnedProfile(UMaterialInstanceConstant* Material)
{
	if (const UOmniImportMaterialUserData* Ownership = Cast<UOmniImportMaterialUserData>(Material->GetAssetUserDataOfClass(UOmniImportMaterialUserData::StaticClass())))
		return Ownership->OwnedToonProfile;
	const FString OwnedPath = Material->GetOutermost()->GetMetaData().GetValue(Material, OwnedProfileKey);
	return OwnedPath.IsEmpty() ? nullptr : Cast<UToonProfile>(FSoftObjectPath(OwnedPath).TryLoad());
}

TArray<FMaterialParameterInfo> GetToonProfileParameters(const UMaterialInterface* Material)
{
	TArray<FMaterialParameterInfo> Parameters;
	TArray<FGuid> ParameterIds;
	Material->GetAllToonProfileParameterInfo(Parameters, ParameterIds);
	return Parameters;
}

UToonProfile* GetLegacyToonProfile(const UMaterialInstance* Material)
{
	return Material && Material->bOverrideToonProfile_DEPRECATED ? Material->ToonProfileOverride_DEPRECATED.Get() : nullptr;
}

void ClearLegacyToonProfile(UMaterialInstanceConstant* Material)
{
	Material->ToonProfileOverride_DEPRECATED = nullptr;
	Material->bOverrideToonProfile_DEPRECATED = false;
}

bool MigrateLegacyToonProfile(UMaterialInstanceConstant* Material)
{
	UToonProfile* Profile = GetLegacyToonProfile(Material);
	if (!Profile) return false;
	const TArray<FMaterialParameterInfo> Parameters = GetToonProfileParameters(Material);
	if (Parameters.IsEmpty()) return false;

	// 5.8 的全局覆盖迁到母材质声明的参数。已存在的 5.9 参数覆盖优先。
	Material->Modify();
	for (const FMaterialParameterInfo& Parameter : Parameters)
	{
		const bool bHasLocalOverride = Material->ToonProfileParameterValues.ContainsByPredicate(
			[&Parameter](const FToonProfileParameterValue& Value) { return Value.ParameterInfo == Parameter && Value.IsValid(); });
		if (!bHasLocalOverride) Material->SetToonProfileParameterValueEditorOnly(Parameter, Profile);
	}
	ClearLegacyToonProfile(Material);
	return true;
}
}

UToonProfile* ResolveBodyToonProfileTemplate(UMaterialInterface* Parent, UToonProfile* ExplicitTemplate)
{
	if (ExplicitTemplate) return ExplicitTemplate;
	if (!Parent) return nullptr;
	const TArray<FMaterialParameterInfo> Parameters = GetToonProfileParameters(Parent);
	if (Parameters.IsEmpty()) return nullptr;
	if (const UMaterialInstance* Instance = Cast<UMaterialInstance>(Parent); Instance && Instance->ToonProfileParameterValues.IsEmpty())
	{
		if (UToonProfile* Legacy = GetLegacyToonProfile(Instance)) return Legacy;
	}
	TSet<UToonProfile*> Profiles;
	for (const FMaterialParameterInfo& Parameter : Parameters)
	{
		UToonProfile* Profile = nullptr;
		if (Parent->GetToonProfileParameterValue(Parameter, Profile) && Profile) Profiles.Add(Profile);
	}
	return Profiles.Num() == 1 ? *Profiles.CreateConstIterator() : nullptr;
}

bool SetOwnedToonProfile(UMaterialInstanceConstant* Material, UToonProfile* Profile, const FString& GroupKey)
{
	if (!Material || !Profile) return false;
	const TArray<FMaterialParameterInfo> Parameters = GetToonProfileParameters(Material);
	if (Parameters.IsEmpty()) return false;

	Material->Modify();
	for (const FMaterialParameterInfo& Parameter : Parameters)
		Material->SetToonProfileParameterValueEditorOnly(Parameter, Profile);
	ClearLegacyToonProfile(Material);
	UOmniImportMaterialUserData* Ownership = Cast<UOmniImportMaterialUserData>(Material->GetAssetUserDataOfClass(UOmniImportMaterialUserData::StaticClass()));
	if (!Ownership)
	{
		Ownership = NewObject<UOmniImportMaterialUserData>(Material, NAME_None, RF_Transactional);
		Material->AddAssetUserData(Ownership);
	}
	Ownership->Modify();
	Ownership->OwnedToonProfile = Profile;
	Ownership->SharedProfileGroupKey = GroupKey;
	// 新绑定改用对象引用记录所有权，移除旧版字符串记录。
	Material->GetOutermost()->GetMetaData().RemoveValue(Material, OwnedProfileKey);
	UMaterialEditingLibrary::UpdateMaterialInstance(Material);
	return true;
}

void ClearOwnedToonProfile(UMaterialInstanceConstant* Material)
{
	if (!Material) return;
	const bool bMigratedLegacy = MigrateLegacyToonProfile(Material);
	FMetaData& Metadata = Material->GetOutermost()->GetMetaData();
	const UOmniImportMaterialUserData* Ownership = Cast<UOmniImportMaterialUserData>(Material->GetAssetUserDataOfClass(UOmniImportMaterialUserData::StaticClass()));
	const FString OwnedPath = Metadata.GetValue(Material, OwnedProfileKey);
	if (!Ownership && OwnedPath.IsEmpty())
	{
		if (bMigratedLegacy) UMaterialEditingLibrary::UpdateMaterialInstance(Material);
		return;
	}
	UToonProfile* OwnedProfile = FindOwnedProfile(Material);
	Material->Modify();
	// 比较对象引用，既支持资产重命名，也保留用户手动替换的 Profile。
	// 旧记录可通过重定向器解析；无法解析旧路径时保留现有覆盖。
	if (OwnedProfile && GetLegacyToonProfile(Material) == OwnedProfile) ClearLegacyToonProfile(Material);
	const int32 Removed = OwnedProfile ? Material->ToonProfileParameterValues.RemoveAll(
		[OwnedProfile](const FToonProfileParameterValue& Value) { return Value.ParameterValue == OwnedProfile; }) : 0;
	Material->RemoveUserDataOfClass(UOmniImportMaterialUserData::StaticClass());
	Metadata.RemoveValue(Material, OwnedProfileKey);
	if (Removed > 0 || bMigratedLegacy) UMaterialEditingLibrary::UpdateMaterialInstance(Material);
	else (void)Material->MarkPackageDirty();
}

UToonProfile* ResolveOrCreateSharedToonProfile(const FString& Package, const TArray<UMaterialInstanceConstant*>& ExistingMIs,
	UToonProfile* Template, const FImportReporter& Report)
{
	if (!Template) return nullptr;
	if (!FPackageName::IsValidLongPackageName(Package))
	{
		Report(TEXT("Shared Toon Profile package is invalid: ") + Package, true);
		return nullptr;
	}
	TSet<UToonProfile*> OwnedProfiles;
	for (UMaterialInstanceConstant* Material : ExistingMIs)
	{
		if (!Material) continue;
		const UOmniImportMaterialUserData* Ownership = Cast<UOmniImportMaterialUserData>(Material->GetAssetUserDataOfClass(UOmniImportMaterialUserData::StaticClass()));
		// 旧版单材质实例记录不含组标识，不能据此选作整组共用 Profile。
		if (Ownership && Ownership->SharedProfileGroupKey.Equals(Package, ESearchCase::IgnoreCase) && Ownership->OwnedToonProfile)
			OwnedProfiles.Add(Ownership->OwnedToonProfile.Get());
	}
	if (OwnedProfiles.Num() > 1)
	{
		Report(TEXT("Multiple owned Toon Profiles disagree for import group: ") + Package, true);
		return nullptr;
	}
	if (OwnedProfiles.Num() == 1) return *OwnedProfiles.CreateConstIterator();
	const FString Folder = FPackageName::GetLongPackagePath(Package);
	const FString Name = FPackageName::GetLongPackageAssetName(Package);
	const FString ObjectPath = Package + TEXT(".") + Name;
	UObject* Existing = StaticFindObject(UObject::StaticClass(), nullptr, *ObjectPath);
	if (!Existing && FPackageName::DoesPackageExist(Package)) Existing = LoadObject<UObject>(nullptr, *ObjectPath);
	UToonProfile* ModelProfile = Cast<UToonProfile>(Existing);
	if (!ModelProfile && (Existing || FPackageName::DoesPackageExist(Package)))
	{
		Report(TEXT("Shared Toon Profile destination is occupied: ") + ObjectPath, true);
		return nullptr;
	}
	if (!ModelProfile)
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();
		ModelProfile = Cast<UToonProfile>(AssetTools.DuplicateAsset(Name, Folder, Template));
	}
	if (!ModelProfile)
	{
		Report(TEXT("Could not create shared Toon Profile: ") + ObjectPath, true);
		return nullptr;
	}
	return ModelProfile;
}

void ApplySharedToonProfile(UMaterialInstanceConstant* Material, UToonProfile* SharedProfile, const FString& GroupKey,
	const FImportReporter& Report)
{
	if (!Material) return;
	if (!SharedProfile)
	{
		ClearOwnedToonProfile(Material);
		return;
	}
	if (GroupKey.IsEmpty())
	{
		Report(TEXT("Shared Toon Profile assignment requires an import group key: ") + Material->GetPathName(), true);
		return;
	}
	UToonProfile* OwnedProfile = FindOwnedProfile(Material);
	const bool bMigratedLegacy = MigrateLegacyToonProfile(Material);
	if (Material->ToonProfileParameterValues.ContainsByPredicate(
		[OwnedProfile](const FToonProfileParameterValue& Value) { return Value.IsValid() && Value.ParameterValue != OwnedProfile; }))
	{
		if (bMigratedLegacy) UMaterialEditingLibrary::UpdateMaterialInstance(Material);
		Report(TEXT("Preserved user-assigned Toon Profile: ") + Material->GetPathName());
		return;
	}
	if (!SetOwnedToonProfile(Material, SharedProfile, GroupKey))
	{
		ClearOwnedToonProfile(Material);
		return;
	}
	Report(TEXT("Assigned group-shared Toon Profile: ") + Material->GetPathName() + TEXT(" -> ") + SharedProfile->GetPathName());
}
}
