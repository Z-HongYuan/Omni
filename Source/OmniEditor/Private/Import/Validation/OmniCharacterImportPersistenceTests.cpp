// Copyright © 2026 张鸿源. All Rights Reserved.
#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Import/OmniCharacterImportPipeline.h"
#include "Import/Materials/OmniImportMaterialUserData.h"
#include "Import/Materials/OmniImportToonProfile.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/ToonProfile.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "HAL/FileManager.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/CommandLine.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/ScopeExit.h"
#include "UObject/Package.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOmniImportSavedMaterialProfileTest,
	"Omni.Editor.CharacterImportPersistence.SavedMaterialProfile",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FOmniImportSavedMaterialProfileTest::RunTest(const FString& Parameters)
{
	FString Root;
	FString SecondRoot;
	FString Content;
	if (!FParse::Value(FCommandLine::Get(), TEXT("OmniImportFixtureRoot="), Root) ||
		!FParse::Value(FCommandLine::Get(), TEXT("OmniImportFixtureContent="), Content))
	{
		AddError(TEXT("Supply -OmniImportFixtureRoot and -OmniImportFixtureContent for a saved Misc fixture."));
		return false;
	}
	FParse::Value(FCommandLine::Get(), TEXT("OmniImportFixtureSecondRoot="), SecondRoot);
	auto IsFixtureRoot = [](FString& Value)
	{
		Value.RemoveFromEnd(TEXT("/"));
		for (const FString Prefix : {TEXT("/OmniImportTests/Batch20261016_"), TEXT("/OmniImportTests/SharedTP20261017_")})
		{
			if (Value.StartsWith(Prefix) && Value.Len() > Prefix.Len() && !Value.Contains(TEXT("..")) &&
				!Value.Contains(TEXT("\\")) && !Value.Mid(Prefix.Len()).Contains(TEXT("/"))) return true;
		}
		return false;
	};
	if (!IsFixtureRoot(Root) || (!SecondRoot.IsEmpty() && (!IsFixtureRoot(SecondRoot) || SecondRoot == Root)) || FPaths::IsRelative(Content))
	{
		AddError(TEXT("Fixture roots must be distinct /OmniImportTests/Batch20261016_<id> or SharedTP20261017_<id> groups; Content must be absolute."));
		return false;
	}
	Content = FPaths::ConvertRelativePathToFull(Content);
	FPaths::NormalizeDirectoryName(Content);
	FPaths::CollapseRelativeDirectories(Content);
	FString MiscRoot = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("../Misc"));
	FPaths::NormalizeDirectoryName(MiscRoot);
	FPaths::CollapseRelativeDirectories(MiscRoot);
	if (!FPaths::IsUnderDirectory(Content, MiscRoot) || !IFileManager::Get().DirectoryExists(*Content))
	{
		AddError(TEXT("Fixture Content must be an existing directory inside this workspace's Misc directory."));
		return false;
	}
	const FString Mount = TEXT("/OmniImportTests/");
	const bool bRegisterMount = !FPackageName::MountPointExists(Mount);
	if (!bRegisterMount && !FPaths::IsSamePath(FPackageName::LongPackageNameToFilename(Mount), Content))
	{
		AddError(TEXT("An existing /OmniImportTests mount points elsewhere; no mount was replaced."));
		return false;
	}
	if (bRegisterMount) FPackageName::RegisterMountPoint(Mount, Content + TEXT("/"));
	ON_SCOPE_EXIT
	{
		if (bRegisterMount) FPackageName::UnRegisterMountPoint(Mount, Content + TEXT("/"));
	};
	UOmniCharacterImportPipeline* Preset = LoadObject<UOmniCharacterImportPipeline>(nullptr,
		TEXT("/Game/Editor/Import/P_OmniCharacterImport.P_OmniCharacterImport"));
	if (!TestNotNull(TEXT("The saved formal import preset is available"), Preset)) return false;
	UMaterialInterface* BodyParent = Preset->BodyParentMaterial.LoadSynchronous();
	if (!TestNotNull(TEXT("The formal Body parent resolves"), BodyParent)) return false;
	UToonProfile* Template = Omni::CharacterImport::ResolveBodyToonProfileTemplate(BodyParent, Preset->BodyToonProfileTemplate.LoadSynchronous());
	if (!TestNotNull(TEXT("The formal group template resolves"), Template)) return false;
	IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	auto ValidateGroup = [&](const FString& GroupRoot) -> UToonProfile*
	{
		Registry.ScanPathsSynchronous({GroupRoot}, true);
		TArray<FAssetData> Assets;
		Registry.GetAssetsByPath(FName(*GroupRoot), Assets, true, true);
		int32 MaterialCount = 0;
		int32 ProfileCount = 0;
		int32 MeshCount = 0;
		UToonProfile* Shared = nullptr;
		UToonProfile* ListedProfile = nullptr;
		FString GroupKey;
		for (const FAssetData& Asset : Assets)
		{
			if (Asset.AssetClassPath == UToonProfile::StaticClass()->GetClassPathName())
			{
				++ProfileCount;
				ListedProfile = Cast<UToonProfile>(Asset.GetAsset());
			}
			if (Asset.AssetClassPath != UMaterialInstanceConstant::StaticClass()->GetClassPathName()) continue;
			++MaterialCount;
			UMaterialInstanceConstant* Material = Cast<UMaterialInstanceConstant>(Asset.GetAsset());
			if (!TestNotNull(*FString::Printf(TEXT("Saved MI loads: %s"), *Asset.GetObjectPathString()), Material)) continue;
			// 即使 Body、Face、Outline 使用不同母材质，也应共用组 Profile。
			TestNotNull(*FString::Printf(TEXT("Saved MI retains a valid parent: %s"), *Material->GetName()), Material->Parent.Get());
			TestTrue(*FString::Printf(TEXT("Saved MI enables Toon override: %s"), *Material->GetName()), Material->bOverrideToonProfile != 0);
			UToonProfile* Override = Material->ToonProfileOverride;
			if (TestNotNull(*FString::Printf(TEXT("Saved Toon override loads: %s"), *Material->GetName()), Override))
			{
				TestNotEqual(TEXT("The group override is a copy, not the source template"), Override, Template);
				TestTrue(TEXT("The copied Profile remains inside this fixture group"), Override->GetOutermost()->GetName().StartsWith(GroupRoot + TEXT("/")));
				if (!Shared) Shared = Override;
				TestEqual(TEXT("Every saved MI references the exact same shared Profile object"), Override, Shared);
			}
			const UOmniImportMaterialUserData* Ownership = Cast<UOmniImportMaterialUserData>(Material->GetAssetUserDataOfClass(UOmniImportMaterialUserData::StaticClass()));
			if (TestNotNull(*FString::Printf(TEXT("Saved ownership loads: %s"), *Material->GetName()), Ownership))
			{
				TestTrue(TEXT("The ownership record remains editor-only"), Ownership->IsEditorOnly());
				TestEqual(TEXT("Saved ownership references the exact applied shared Profile"), Ownership->OwnedToonProfile.Get(), Override);
				TestFalse(TEXT("Saved group ownership has a nonempty key"), Ownership->SharedProfileGroupKey.IsEmpty());
				TestTrue(TEXT("The saved group key belongs to this fixture group"), Ownership->SharedProfileGroupKey.StartsWith(GroupRoot + TEXT("/")));
				TestTrue(TEXT("The saved group key is a valid package identity"), FPackageName::IsValidLongPackageName(Ownership->SharedProfileGroupKey));
				if (GroupKey.IsEmpty()) GroupKey = Ownership->SharedProfileGroupKey;
				TestTrue(TEXT("Every saved MI has the same group ownership key"), Ownership->SharedProfileGroupKey.Equals(GroupKey, ESearchCase::IgnoreCase));
			}
		}
		TestTrue(TEXT("Each saved group contains at least two material instances"), MaterialCount >= 2);
		TestEqual(TEXT("Each saved group contains exactly one generated Toon Profile"), ProfileCount, 1);
		TestEqual(TEXT("The one generated Profile is the object referenced by all MIs"), ListedProfile, Shared);
		auto CheckSlot = [&](UMaterialInterface* Slot)
		{
			UMaterialInstanceConstant* Material = Cast<UMaterialInstanceConstant>(Slot);
			if (TestNotNull(TEXT("Imported mesh slots reference material instances"), Material))
			{
				TestTrue(TEXT("Mesh slot MI belongs to this fixture group"), Material->GetOutermost()->GetName().StartsWith(GroupRoot + TEXT("/")));
				TestEqual(TEXT("Every mesh slot's MI uses the group shared Profile"), Material->ToonProfileOverride.Get(), Shared);
			}
		};
		for (const FAssetData& Asset : Assets)
		{
			if (Asset.AssetClassPath == USkeletalMesh::StaticClass()->GetClassPathName())
			{
				if (const USkeletalMesh* Mesh = Cast<USkeletalMesh>(Asset.GetAsset()))
				{
					++MeshCount;
					for (const FSkeletalMaterial& Slot : Mesh->GetMaterials()) CheckSlot(Slot.MaterialInterface);
				}
			}
			else if (Asset.AssetClassPath == UStaticMesh::StaticClass()->GetClassPathName())
			{
				if (const UStaticMesh* Mesh = Cast<UStaticMesh>(Asset.GetAsset()))
				{
					++MeshCount;
					for (const FStaticMaterial& Slot : Mesh->GetStaticMaterials()) CheckSlot(Slot.MaterialInterface);
				}
			}
		}
		TestTrue(TEXT("The saved import group contains at least one mesh"), MeshCount > 0);
		AddInfo(FString::Printf(TEXT("Read %d MIs, %d Toon Profile and %d meshes under %s; no assets were changed or saved."), MaterialCount, ProfileCount, MeshCount, *GroupRoot));
		return Shared;
	};
	UToonProfile* First = ValidateGroup(Root);
	if (!SecondRoot.IsEmpty())
	{
		UToonProfile* Second = ValidateGroup(SecondRoot);
		if (First && Second) TestNotEqual(TEXT("Two saved FBX import groups use distinct Profile objects"), First, Second);
	}
	return !HasAnyErrors();
}

#endif
