// Copyright © 2026 张鸿源. All Rights Reserved.
#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Import/OmniCharacterImportPipeline.h"
#include "Import/OmniCharacterImportProfile.h"
#include "Import/Materials/OmniImportMaterialUserData.h"
#include "Import/Materials/OmniImportToonProfile.h"
#include "Engine/ToonProfile.h"
#include "Engine/SkeletalMesh.h"
#include "InterchangeMaterialFactoryNode.h"
#include "InterchangeMaterialInstanceNode.h"
#include "InterchangeSkeletalMeshFactoryNode.h"
#include "InterchangeSourceData.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/Guid.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Nodes/InterchangeBaseNodeContainer.h"
#include "UObject/Package.h"

namespace
{
constexpr EAutomationTestFlags MaterialTestFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

template <typename T>
T* AddMaterialTestNode(UInterchangeBaseNodeContainer* Container, const FString& Uid, const FString& Label, EInterchangeNodeContainerType Type)
{
	T* Node = NewObject<T>(Container);
	Node->InitializeNode(Uid, Label, Type);
	Container->AddNode(Node);
	return Node;
}

/** 仅驻留内存的临时测试组；预建目标 TP，避免触发 AssetTools 保存。 */
struct FMemoryProfileGroup
{
	FString Id = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	FString Mount = TEXT("/OmniImportMemory_") + Id + TEXT("/");
	FString Content = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("../Misc/validation/CharacterImport/SharedTP20261017/Memory") / Id) + TEXT("/");
	FString Folder = Mount + TEXT("Group");
	FString GroupKey = Folder / TEXT("Materials/TP_Group");
	TArray<UPackage*> Packages;
	TArray<UMaterialInstanceConstant*> Materials;
	UToonProfile* Template = nullptr;
	UToonProfile* Shared = nullptr;

	explicit FMemoryProfileGroup(int32 MaterialCount = 3)
	{
		// 使用进程内私有挂载点满足目标路径验证；所有包保持临时且未保存，避免依赖只读的 /Temp 根。
		FPackageName::RegisterMountPoint(Mount, Content);
		Template = NewObject<UToonProfile>(GetTransientPackage(), NAME_None, RF_Transient);
		Template->Settings.DiffuseRampOffsetStrength = 0.25f;
		Shared = DuplicateObject<UToonProfile>(Template, MakePackage(GroupKey), TEXT("TP_Group"));
		Shared->SetFlags(RF_Transient);
		for (int32 Index = 0; Index < MaterialCount; ++Index)
		{
			const FString Name = FString::Printf(TEXT("MI_Part%d"), Index);
			Materials.Add(NewObject<UMaterialInstanceConstant>(MakePackage(Folder / Name), FName(*Name), RF_Transient));
		}
	}

	~FMemoryProfileGroup()
	{
		for (UPackage* Package : Packages) Package->SetDirtyFlag(false);
		FPackageName::UnRegisterMountPoint(Mount, Content);
	}

	UPackage* MakePackage(const FString& Name)
	{
		UPackage* Package = CreatePackage(*Name);
		Package->SetFlags(RF_Transient);
		Packages.Add(Package);
		return Package;
	}

	UToonProfile* MakeProfile(const FString& Name)
	{
		return NewObject<UToonProfile>(MakePackage(Folder / Name), FName(*Name), RF_Transient);
	}
};

const UOmniImportMaterialUserData* ProfileOwnership(UMaterialInstanceConstant* Material)
{
	return Cast<UOmniImportMaterialUserData>(Material->GetAssetUserDataOfClass(UOmniImportMaterialUserData::StaticClass()));
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOmniImportCustomParentTest, "Omni.Editor.CharacterImport.CustomMaterialParent", MaterialTestFlags)
bool FOmniImportCustomParentTest::RunTest(const FString& Parameters)
{
	UMaterial* Parent = NewObject<UMaterial>(GetTransientPackage(), NAME_None, RF_Transient);
	TestNull(TEXT("The custom fixture has no Toon Profile"), Omni::CharacterImport::ResolveBodyToonProfileTemplate(Parent, nullptr));
	UOmniCharacterImportPipeline* Pipeline = NewObject<UOmniCharacterImportPipeline>();
	UOmniCharacterImportProfile* Profile = NewObject<UOmniCharacterImportProfile>();
	Profile->bCreateIKRig = false;
	Profile->bCreatePostProcessAnimBlueprint = false;
	Pipeline->Profile = Profile;
	Pipeline->BodyParentMaterial = nullptr; // 仅源材质专属覆盖可提供有效母材质。
	Pipeline->bNormalizeTextureNames = false;
	FOmniKeywordMaterialBinding& Section = Pipeline->MaterialMappings.AddDefaulted_GetRef();
	Section.SourceMaterial = TEXT("Fixture_Body");
	Section.ParentOverride = Parent;
	UInterchangeBaseNodeContainer* Container = NewObject<UInterchangeBaseNodeContainer>();
	AddMaterialTestNode<UInterchangeSkeletalMeshFactoryNode>(Container, TEXT("Mesh"), TEXT("Fixture"), EInterchangeNodeContainerType::FactoryData);
	AddMaterialTestNode<UInterchangeMaterialInstanceNode>(Container, TEXT("Body"), TEXT("Fixture_Body"), EInterchangeNodeContainerType::TranslatedAsset);
	const FString FactoryUid = UInterchangeFactoryBaseNode::BuildFactoryNodeUid(TEXT("Body"));
	UInterchangeMaterialInstanceFactoryNode* Factory = AddMaterialTestNode<UInterchangeMaterialInstanceFactoryNode>(
		Container, FactoryUid, TEXT("Fixture_Body"), EInterchangeNodeContainerType::FactoryData);
	FInterchangePipelineContextParams Context;
	Context.ContextType = EInterchangePipelineContext::AssetImport;
	Context.BaseNodeContainer = Container;
	Pipeline->AdjustSettingsForContext(Context);
	TOptional<FText> Reason;
	TestTrue(TEXT("A custom Body parent without Toon is valid"), Pipeline->IsSettingsAreValid(Reason));
	if (!TestEqual(TEXT("One source material remains independently configurable"), Pipeline->MaterialMappings.Num(), 1)) return false;
	TestEqual(TEXT("The override wins over the role default"), Pipeline->MaterialMappings[0].Parent.Get(), static_cast<UMaterialInterface*>(Parent));
	UInterchangeSourceData* Source = NewObject<UInterchangeSourceData>();
	Source->SetFilename(TEXT("CustomMaterialParent.fbx"));
	const FString Destination = TEXT("/Game/OmniAutomation/") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
	Pipeline->ScriptedExecutePipeline(Container, {Source}, Destination);
	TestTrue(TEXT("A plain custom parent does not disable material import"), Factory->IsEnabled());
	FString ConfiguredParent;
	TestTrue(TEXT("The MI factory has a configured parent"), Factory->GetCustomParent(ConfiguredParent));
	TestEqual(TEXT("The MI factory uses the custom parent"), ConfiguredParent, Parent->GetPathName());
	UMaterialInstanceConstant* Material = NewObject<UMaterialInstanceConstant>(GetTransientPackage(), NAME_None, RF_Transient);
	// 只触发材质回调，避免测试创建 Rig、网格体报告或已保存资产。
	Pipeline->ScriptedExecutePostImportPipeline(Container, FactoryUid, Material, false);
	TestEqual(TEXT("The created MI actually uses the selected custom parent"), Material->Parent.Get(), static_cast<UMaterialInterface*>(Parent));
	TestFalse(TEXT("A non-Toon parent does not gain a Toon override"), Material->bOverrideToonProfile != 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOmniImportRenamedProfileOwnershipTest, "Omni.Editor.CharacterImport.RenamedProfileOwnership", MaterialTestFlags)
bool FOmniImportRenamedProfileOwnershipTest::RunTest(const FString& Parameters)
{
	UMaterialInstanceConstant* Material = NewObject<UMaterialInstanceConstant>(GetTransientPackage(), NAME_None, RF_Transient);
	UToonProfile* Owned = NewObject<UToonProfile>(GetTransientPackage(), NAME_None, RF_Transient);
	Omni::CharacterImport::SetOwnedToonProfile(Material, Owned);
	const UOmniImportMaterialUserData* Ownership = Cast<UOmniImportMaterialUserData>(Material->GetAssetUserDataOfClass(UOmniImportMaterialUserData::StaticClass()));
	if (!TestNotNull(TEXT("Assignments record ownership as an object reference"), Ownership)) return false;
	TestTrue(TEXT("The ownership record is editor-only"), Ownership->IsEditorOnly());
	const FString PreviousPath = Owned->GetPathName();
	const FName NewName = MakeUniqueObjectName(GetTransientPackage(), UToonProfile::StaticClass(), TEXT("RenamedImportedProfile"));
	if (!TestTrue(TEXT("The disposable profile can be renamed without a redirector"),
		Owned->Rename(*NewName.ToString(), GetTransientPackage(), REN_DontCreateRedirectors | REN_DoNotDirty | REN_NonTransactional))) return false;
	TestNotEqual(TEXT("The profile path changed"), Owned->GetPathName(), PreviousPath);
	TestEqual(TEXT("Ownership still identifies the renamed object"), Ownership->OwnedToonProfile.Get(), Owned);
	Omni::CharacterImport::ClearOwnedToonProfile(Material);
	TestNull(TEXT("Renaming does not prevent removal of an owned override"), Material->ToonProfileOverride.Get());
	TestFalse(TEXT("The owned override is disabled"), Material->bOverrideToonProfile != 0);
	TestNull(TEXT("Clearing also removes the ownership record"), Material->GetAssetUserDataOfClass(UOmniImportMaterialUserData::StaticClass()));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOmniImportUserProfileProtectionTest, "Omni.Editor.CharacterImport.UserProfileProtection", MaterialTestFlags)
bool FOmniImportUserProfileProtectionTest::RunTest(const FString& Parameters)
{
	UMaterialInstanceConstant* Material = NewObject<UMaterialInstanceConstant>(GetTransientPackage(), NAME_None, RF_Transient);
	UToonProfile* Owned = NewObject<UToonProfile>(GetTransientPackage(), NAME_None, RF_Transient);
	UToonProfile* Manual = NewObject<UToonProfile>(GetTransientPackage(), NAME_None, RF_Transient);
	UToonProfile* Template = NewObject<UToonProfile>(GetTransientPackage(), NAME_None, RF_Transient);
	Omni::CharacterImport::SetOwnedToonProfile(Material, Owned);
	Material->ToonProfileOverride = Manual;
	int32 Errors = 0;
	const Omni::CharacterImport::FImportReporter Reporter{[&Errors](const FString&, bool bError) { Errors += bError ? 1 : 0; }};
	const FString GroupKey = TEXT("/Temp/OmniImportMaterialTests/UserReplacement/TP_Group");
	Omni::CharacterImport::ApplySharedToonProfile(Material, Template, GroupKey, Reporter);
	TestEqual(TEXT("Reapplying a template preserves the user's replacement"), Material->ToonProfileOverride.Get(), Manual);
	Omni::CharacterImport::ApplySharedToonProfile(Material, nullptr, GroupKey, Reporter);
	TestEqual(TEXT("Switching to a parent without Toon preserves the user's replacement"), Material->ToonProfileOverride.Get(), Manual);
	TestTrue(TEXT("The user override remains enabled"), Material->bOverrideToonProfile != 0);
	TestNull(TEXT("The old ownership record is removed"), Material->GetAssetUserDataOfClass(UOmniImportMaterialUserData::StaticClass()));
	TestEqual(TEXT("Preserving a user assignment is not an import error"), Errors, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOmniImportMaterialProfileReuseTest, "Omni.Editor.CharacterImport.MaterialProfileReuse", MaterialTestFlags)
bool FOmniImportMaterialProfileReuseTest::RunTest(const FString& Parameters)
{
	FMemoryProfileGroup Group;
	int32 Errors = 0;
	const Omni::CharacterImport::FImportReporter Reporter{[&Errors](const FString&, bool bError) { Errors += bError ? 1 : 0; }};
	UToonProfile* Shared = Omni::CharacterImport::ResolveOrCreateSharedToonProfile(Group.GroupKey, Group.Materials, Group.Template, Reporter);
	if (!TestEqual(TEXT("The import group resolves its local copy"), Shared, Group.Shared)) return false;
	TestNotEqual(TEXT("The group uses a copy, not the shared source template"), Shared, Group.Template);
	for (UMaterialInstanceConstant* Material : Group.Materials)
	{
		Omni::CharacterImport::ApplySharedToonProfile(Material, Shared, Group.GroupKey, Reporter);
		TestEqual(TEXT("All three MIs reference the exact same Profile object"), Material->ToonProfileOverride.Get(), Shared);
		TestTrue(TEXT("Every MI enables the shared override"), Material->bOverrideToonProfile != 0);
		const UOmniImportMaterialUserData* Ownership = ProfileOwnership(Material);
		if (TestNotNull(TEXT("Every MI has a persistent ownership record"), Ownership))
		{
			TestEqual(TEXT("Ownership references that same shared object"), Ownership->OwnedToonProfile.Get(), Shared);
			TestEqual(TEXT("Ownership identifies the FBX import group"), Ownership->SharedProfileGroupKey, Group.GroupKey);
		}
	}
	Shared->Settings.DiffuseRampOffsetStrength = 0.75f;
	UToonProfile* Again = Omni::CharacterImport::ResolveOrCreateSharedToonProfile(Group.GroupKey, Group.Materials, Group.Template, Reporter);
	if (!TestEqual(TEXT("Repeated material/mesh callbacks reuse one group object"), Again, Shared)) return false;
	TestEqual(TEXT("Group tuning survives another resolution"), Again->Settings.DiffuseRampOffsetStrength, 0.75f);
	TestEqual(TEXT("Editing the group's copy never edits its source template"), Group.Template->Settings.DiffuseRampOffsetStrength, 0.25f);
	TestEqual(TEXT("Valid shared assignments report no errors"), Errors, 0);
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOmniImportSharedProfileIsolationTest, "Omni.Editor.CharacterImport.SharedProfileGroupIsolation", MaterialTestFlags)
bool FOmniImportSharedProfileIsolationTest::RunTest(const FString& Parameters)
{
	FMemoryProfileGroup First(2);
	FMemoryProfileGroup Second(2);
	int32 Errors = 0;
	const Omni::CharacterImport::FImportReporter Reporter{[&Errors](const FString&, bool bError) { Errors += bError ? 1 : 0; }};
	for (UMaterialInstanceConstant* Material : First.Materials)
		Omni::CharacterImport::ApplySharedToonProfile(Material, First.Shared, First.GroupKey, Reporter);
	// 其他源组的候选材质不能使当前组选用其 TP。
	UToonProfile* Other = Omni::CharacterImport::ResolveOrCreateSharedToonProfile(Second.GroupKey, First.Materials, First.Template, Reporter);
	TestEqual(TEXT("Another FBX group resolves its own local TP"), Other, Second.Shared);
	TestNotEqual(TEXT("Two groups never share their generated Profile"), Other, First.Shared);
	for (UMaterialInstanceConstant* Material : Second.Materials)
		Omni::CharacterImport::ApplySharedToonProfile(Material, Other, Second.GroupKey, Reporter);
	Second.Shared->Settings.DiffuseRampOffsetStrength = 0.8f;
	TestEqual(TEXT("Tuning the second group does not affect the first"), First.Shared->Settings.DiffuseRampOffsetStrength, 0.25f);
	TestEqual(TEXT("Tuning either group does not edit the source template"), First.Template->Settings.DiffuseRampOffsetStrength, 0.25f);
	for (UMaterialInstanceConstant* Material : Second.Materials)
		TestEqual(TEXT("Both second-group MIs reference its one Profile"), Material->ToonProfileOverride.Get(), Second.Shared);
	TestEqual(TEXT("Independent groups report no errors"), Errors, 0);
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOmniImportLegacyProfileMigrationTest, "Omni.Editor.CharacterImport.LegacyOwnershipDoesNotSelectGroup", MaterialTestFlags)
bool FOmniImportLegacyProfileMigrationTest::RunTest(const FString& Parameters)
{
	FMemoryProfileGroup Group(2);
	UToonProfile* Legacy = Group.MakeProfile(TEXT("TP_LegacyPerMI"));
	Legacy->Settings.DiffuseRampOffsetStrength = 0.6f;
	Omni::CharacterImport::SetOwnedToonProfile(Group.Materials[0], Legacy); // 不提供共用组标识。
	int32 Errors = 0;
	const Omni::CharacterImport::FImportReporter Reporter{[&Errors](const FString&, bool bError) { Errors += bError ? 1 : 0; }};
	UToonProfile* Shared = Omni::CharacterImport::ResolveOrCreateSharedToonProfile(Group.GroupKey, Group.Materials, Group.Template, Reporter);
	TestEqual(TEXT("Legacy per-MI ownership cannot become the group Profile"), Shared, Group.Shared);
	TestNotEqual(TEXT("The old per-MI object is not silently reused"), Shared, Legacy);
	for (UMaterialInstanceConstant* Material : Group.Materials)
		Omni::CharacterImport::ApplySharedToonProfile(Material, Shared, Group.GroupKey, Reporter);
	TestEqual(TEXT("An owned legacy assignment can migrate to the shared object"), Group.Materials[0]->ToonProfileOverride.Get(), Shared);
	if (const UOmniImportMaterialUserData* Ownership = ProfileOwnership(Group.Materials[0]))
		TestEqual(TEXT("Migration records the new shared group key"), Ownership->SharedProfileGroupKey, Group.GroupKey);
	else AddError(TEXT("Migrated material has no group ownership."));
	TestEqual(TEXT("Migration does not edit or retune the legacy object"), Legacy->Settings.DiffuseRampOffsetStrength, 0.6f);
	TestEqual(TEXT("Legacy migration reports no errors"), Errors, 0);
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOmniImportRenamedSharedProfileTest, "Omni.Editor.CharacterImport.RenamedSharedProfileReuse", MaterialTestFlags)
bool FOmniImportRenamedSharedProfileTest::RunTest(const FString& Parameters)
{
	FMemoryProfileGroup Group;
	int32 Errors = 0;
	const Omni::CharacterImport::FImportReporter Reporter{[&Errors](const FString&, bool bError) { Errors += bError ? 1 : 0; }};
	for (int32 Index = 0; Index < 2; ++Index)
		Omni::CharacterImport::ApplySharedToonProfile(Group.Materials[Index], Group.Shared, Group.GroupKey, Reporter);
	const FString PreviousPath = Group.Shared->GetPathName();
	UPackage* RenamedPackage = Group.MakePackage(Group.Folder / TEXT("Renamed/TP_Retuned"));
	if (!TestTrue(TEXT("The shared Profile can move without a redirector"),
		Group.Shared->Rename(TEXT("TP_Retuned"), RenamedPackage, REN_DontCreateRedirectors | REN_DoNotDirty | REN_NonTransactional))) return false;
	Group.Shared->Settings.DiffuseRampOffsetStrength = 0.85f;
	TestNotEqual(TEXT("The shared Profile's saved path changes"), Group.Shared->GetPathName(), PreviousPath);
	UToonProfile* Again = Omni::CharacterImport::ResolveOrCreateSharedToonProfile(Group.GroupKey, Group.Materials, Group.Template, Reporter);
	if (!TestEqual(TEXT("Group ownership reuses the renamed shared object"), Again, Group.Shared)) return false;
	Omni::CharacterImport::ApplySharedToonProfile(Group.Materials[2], Again, Group.GroupKey, Reporter);
	TestEqual(TEXT("A later MI joins the renamed shared object"), Group.Materials[2]->ToonProfileOverride.Get(), Group.Shared);
	if (const UOmniImportMaterialUserData* Ownership = ProfileOwnership(Group.Materials[2]))
		TestEqual(TEXT("The stable group key survives the Profile move"), Ownership->SharedProfileGroupKey, Group.GroupKey);
	else AddError(TEXT("The later material has no group ownership."));
	TestEqual(TEXT("Renaming does not lose shared tuning"), Again->Settings.DiffuseRampOffsetStrength, 0.85f);
	TestEqual(TEXT("Reusing a renamed group reports no errors"), Errors, 0);
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOmniImportNullSharedTemplateTest, "Omni.Editor.CharacterImport.NullSharedTemplateClearsOnlyOwned", MaterialTestFlags)
bool FOmniImportNullSharedTemplateTest::RunTest(const FString& Parameters)
{
	FMemoryProfileGroup Group;
	UToonProfile* Manual = Group.MakeProfile(TEXT("TP_Manual"));
	Omni::CharacterImport::SetOwnedToonProfile(Group.Materials[0], Group.Shared, Group.GroupKey);
	Group.Materials[1]->ToonProfileOverride = Manual;
	Group.Materials[1]->bOverrideToonProfile = true;
	Omni::CharacterImport::SetOwnedToonProfile(Group.Materials[2], Group.Shared, Group.GroupKey);
	Group.Materials[2]->ToonProfileOverride = Manual;
	int32 Errors = 0;
	const Omni::CharacterImport::FImportReporter Reporter{[&Errors](const FString&, bool bError) { Errors += bError ? 1 : 0; }};
	const FString MissingPackage = Group.Folder / TEXT("NeverCreated/TP_None");
	TestNull(TEXT("The missing destination starts without a package"), FindPackage(nullptr, *MissingPackage));
	UToonProfile* None = Omni::CharacterImport::ResolveOrCreateSharedToonProfile(MissingPackage, Group.Materials, nullptr, Reporter);
	TestNull(TEXT("A null template creates no Profile"), None);
	TestNull(TEXT("A null template creates no package"), FindPackage(nullptr, *MissingPackage));
	TestEqual(TEXT("Resolution alone does not mutate existing assignments"), Group.Materials[0]->ToonProfileOverride.Get(), Group.Shared);
	for (UMaterialInstanceConstant* Material : Group.Materials)
		Omni::CharacterImport::ApplySharedToonProfile(Material, None, Group.GroupKey, Reporter);
	TestNull(TEXT("A former owned assignment is cleared"), Group.Materials[0]->ToonProfileOverride.Get());
	TestFalse(TEXT("A cleared owned assignment is disabled"), Group.Materials[0]->bOverrideToonProfile != 0);
	for (int32 Index = 1; Index < 3; ++Index)
	{
		TestEqual(TEXT("Manual and manually replaced overrides are preserved"), Group.Materials[Index]->ToonProfileOverride.Get(), Manual);
		TestTrue(TEXT("Manual overrides remain enabled"), Group.Materials[Index]->bOverrideToonProfile != 0);
	}
	for (UMaterialInstanceConstant* Material : Group.Materials)
		TestNull(TEXT("Clearing removes only obsolete ownership records"), ProfileOwnership(Material));
	TestEqual(TEXT("No-Toon material handling is not an error"), Errors, 0);
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOmniImportConflictingSharedProfilesTest, "Omni.Editor.CharacterImport.ConflictingSharedProfilesRejected", MaterialTestFlags)
bool FOmniImportConflictingSharedProfilesTest::RunTest(const FString& Parameters)
{
	FMemoryProfileGroup Group(2);
	UToonProfile* Other = Group.MakeProfile(TEXT("TP_Conflicting"));
	Omni::CharacterImport::SetOwnedToonProfile(Group.Materials[0], Group.Shared, Group.GroupKey);
	Omni::CharacterImport::SetOwnedToonProfile(Group.Materials[1], Other, Group.GroupKey);
	int32 Errors = 0;
	const Omni::CharacterImport::FImportReporter Reporter{[&Errors](const FString&, bool bError) { Errors += bError ? 1 : 0; }};
	TestNull(TEXT("Disagreeing group ownership is not resolved by guessing"),
		Omni::CharacterImport::ResolveOrCreateSharedToonProfile(Group.GroupKey, Group.Materials, Group.Template, Reporter));
	TestTrue(TEXT("The conflict is reported as an import error"), Errors > 0);
	TestEqual(TEXT("Rejecting a group leaves its first assignment intact"), Group.Materials[0]->ToonProfileOverride.Get(), Group.Shared);
	TestEqual(TEXT("Rejecting a group leaves its other assignment intact"), Group.Materials[1]->ToonProfileOverride.Get(), Other);
	const FString OccupiedPackage = Group.Folder / TEXT("TP_Occupied");
	UMaterialInstanceConstant* Occupant = NewObject<UMaterialInstanceConstant>(Group.MakePackage(OccupiedPackage), TEXT("TP_Occupied"), RF_Transient);
	Errors = 0;
	TestNull(TEXT("A destination occupied by another asset type is not replaced"),
		Omni::CharacterImport::ResolveOrCreateSharedToonProfile(OccupiedPackage, {}, Group.Template, Reporter));
	TestTrue(TEXT("The occupied destination is reported"), Errors > 0);
	TestEqual(TEXT("The unrelated occupant remains the same object"), FindObject<UMaterialInstanceConstant>(nullptr, *(OccupiedPackage + TEXT(".TP_Occupied"))), Occupant);
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOmniImportSharedProfileCallbackTest, "Omni.Editor.CharacterImport.SharedProfileMaterialCallbacks", MaterialTestFlags)
bool FOmniImportSharedProfileCallbackTest::RunTest(const FString& Parameters)
{
	for (bool bFaceFirst : {false, true})
	{
		FMemoryProfileGroup Group(2);
		UMaterial* BodyParent = NewObject<UMaterial>(GetTransientPackage(), NAME_None, RF_Transient);
		UMaterial* FaceParent = NewObject<UMaterial>(GetTransientPackage(), NAME_None, RF_Transient);
		UOmniCharacterImportPipeline* Pipeline = NewObject<UOmniCharacterImportPipeline>();
		UOmniCharacterImportProfile* Profile = NewObject<UOmniCharacterImportProfile>();
		Profile->bCreateCharacterFolder = false;
		Profile->bCreateIKRig = false;
		Profile->bCreatePostProcessAnimBlueprint = false;
		Pipeline->Profile = Profile;
		Pipeline->CharacterName = TEXT("Group");
		Pipeline->BodyToonProfileTemplate = Group.Template;
		Pipeline->bNormalizeTextureNames = false;
		UInterchangeBaseNodeContainer* Container = NewObject<UInterchangeBaseNodeContainer>();
		AddMaterialTestNode<UInterchangeSkeletalMeshFactoryNode>(Container, TEXT("Mesh"), TEXT("Fixture"), EInterchangeNodeContainerType::FactoryData);
		TArray<FString> FactoryUids;
		for (int32 Index = 0; Index < 2; ++Index)
		{
			const FString SourceUid = Index == 0 ? TEXT("Body") : TEXT("Face");
			const FString Label = TEXT("Fixture_") + SourceUid;
			FOmniKeywordMaterialBinding& Section = Pipeline->MaterialMappings.AddDefaulted_GetRef();
			Section.SourceMaterial = Label;
			Section.Role = Index == 0 ? EOmniMaterialRole::Body : EOmniMaterialRole::Face;
			Section.ParentOverride = Index == 0 ? BodyParent : FaceParent;
			AddMaterialTestNode<UInterchangeMaterialInstanceNode>(Container, SourceUid, Label, EInterchangeNodeContainerType::TranslatedAsset);
			const FString FactoryUid = UInterchangeFactoryBaseNode::BuildFactoryNodeUid(SourceUid);
			FactoryUids.Add(FactoryUid);
			UInterchangeMaterialInstanceFactoryNode* Factory = AddMaterialTestNode<UInterchangeMaterialInstanceFactoryNode>(
				Container, FactoryUid, Label, EInterchangeNodeContainerType::FactoryData);
			Factory->SetCustomReferenceObject(FSoftObjectPath(Group.Materials[Index]));
		}
		FInterchangePipelineContextParams Context;
		Context.ContextType = EInterchangePipelineContext::AssetImport;
		Context.BaseNodeContainer = Container;
		Pipeline->AdjustSettingsForContext(Context);
		UInterchangeSourceData* Source = NewObject<UInterchangeSourceData>();
		Source->SetFilename(TEXT("Group.fbx"));
		Pipeline->ScriptedExecutePipeline(Container, {Source}, Group.Folder);
		// 通过材质回调验证组 Profile 解析与缓存，不创建文件。
		const int32 First = bFaceFirst ? 1 : 0;
		const int32 Second = 1 - First;
		Pipeline->ScriptedExecutePostImportPipeline(Container, FactoryUids[First], Group.Materials[First], false);
		Pipeline->ScriptedExecutePostImportPipeline(Container, FactoryUids[Second], Group.Materials[Second], false);
		TestEqual(TEXT("Body callback preserves its independently selected parent"), Group.Materials[0]->Parent.Get(), static_cast<UMaterialInterface*>(BodyParent));
		TestEqual(TEXT("Face callback preserves its independently selected parent"), Group.Materials[1]->Parent.Get(), static_cast<UMaterialInterface*>(FaceParent));
		for (UMaterialInstanceConstant* Material : Group.Materials)
		{
			TestEqual(TEXT("Both role callbacks share the same group Profile in either order"), Material->ToonProfileOverride.Get(), Group.Shared);
			const UOmniImportMaterialUserData* Ownership = ProfileOwnership(Material);
			if (TestNotNull(TEXT("Callbacks persist group ownership"), Ownership))
				TestEqual(TEXT("Both roles store the same group key"), Ownership->SharedProfileGroupKey, Group.GroupKey);
		}
		Group.Shared->Settings.DiffuseRampOffsetStrength = 0.9f;
		UMaterial* ManualParent = NewObject<UMaterial>(GetTransientPackage(), NAME_None, RF_Transient);
		UToonProfile* ManualProfile = Group.MakeProfile(TEXT("TP_ManualAfterImport"));
		Group.Materials[0]->Parent = ManualParent;
		Group.Materials[0]->ToonProfileOverride = ManualProfile;
		USkeletalMesh* ReimportMesh = NewObject<USkeletalMesh>(GetTransientPackage(), NAME_None, RF_Transient);
		for (UMaterialInstanceConstant* Material : Group.Materials) ReimportMesh->GetMaterials().Add(FSkeletalMaterial(Material));
		Context.ContextType = EInterchangePipelineContext::AssetReimport;
		Context.ReimportAsset = ReimportMesh;
		Pipeline->AdjustSettingsForContext(Context);
		Pipeline->ScriptedExecutePipeline(Container, {Source}, Group.Folder);
		for (int32 Index : {Second, First})
			Pipeline->ScriptedExecutePostImportPipeline(Container, FactoryUids[Index], Group.Materials[Index], true);
		TestEqual(TEXT("Ordinary reimport preserves a hand-edited parent"), Group.Materials[0]->Parent.Get(), static_cast<UMaterialInterface*>(ManualParent));
		TestEqual(TEXT("Ordinary reimport preserves a manual Profile replacement"), Group.Materials[0]->ToonProfileOverride.Get(), ManualProfile);
		TestEqual(TEXT("Ordinary reimport retains the other MI's shared object"), Group.Materials[1]->ToonProfileOverride.Get(), Group.Shared);
		TestEqual(TEXT("Ordinary reimport preserves group tuning"), Group.Shared->Settings.DiffuseRampOffsetStrength, 0.9f);
	}
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOmniImportLegacySharedProfileCallbackTest, "Omni.Editor.CharacterImport.LegacyRuleSharedProfileCallbacks", MaterialTestFlags)
bool FOmniImportLegacySharedProfileCallbackTest::RunTest(const FString& Parameters)
{
	FMemoryProfileGroup Group(2);
	UMaterialInstanceConstant* RuleParent = NewObject<UMaterialInstanceConstant>(GetTransientPackage(), NAME_None, RF_Transient);
	RuleParent->Parent = NewObject<UMaterial>(GetTransientPackage(), NAME_None, RF_Transient);
	RuleParent->ToonProfileOverride = Group.Template;
	RuleParent->bOverrideToonProfile = true;
	UOmniCharacterImportPipeline* Pipeline = NewObject<UOmniCharacterImportPipeline>();
	UOmniCharacterImportProfile* Profile = NewObject<UOmniCharacterImportProfile>();
	Profile->bCreateCharacterFolder = false;
	Profile->bCreateIKRig = false;
	Profile->bCreatePostProcessAnimBlueprint = false;
	Profile->bConfigureMaterials = true;
	FOmniImportMaterialRule& Rule = Profile->MaterialRules.AddDefaulted_GetRef();
	Rule.MaterialNamePattern = TEXT("Legacy_*");
	Rule.ParentMaterial = RuleParent;
	Pipeline->Profile = Profile;
	Pipeline->CharacterName = TEXT("Group");
	Pipeline->bUseKeywordMaterials = false;
	Pipeline->bConfigureFaceMaterial = false;
	Pipeline->bConfigureBodyMaterial = false;
	Pipeline->bNormalizeTextureNames = false;
	// 不指定显式模板，模板只来自通用规则选中的母材质。
	Pipeline->BodyToonProfileTemplate = nullptr;
	UInterchangeBaseNodeContainer* Container = NewObject<UInterchangeBaseNodeContainer>();
	AddMaterialTestNode<UInterchangeSkeletalMeshFactoryNode>(Container, TEXT("Mesh"), TEXT("Fixture"), EInterchangeNodeContainerType::FactoryData);
	TArray<FString> FactoryUids;
	for (int32 Index = 0; Index < 2; ++Index)
	{
		const FString Label = FString::Printf(TEXT("Legacy_%d"), Index);
		AddMaterialTestNode<UInterchangeMaterialInstanceNode>(Container, Label, Label, EInterchangeNodeContainerType::TranslatedAsset);
		const FString FactoryUid = UInterchangeFactoryBaseNode::BuildFactoryNodeUid(Label);
		FactoryUids.Add(FactoryUid);
		UInterchangeMaterialInstanceFactoryNode* Factory = AddMaterialTestNode<UInterchangeMaterialInstanceFactoryNode>(
			Container, FactoryUid, Label, EInterchangeNodeContainerType::FactoryData);
		Factory->SetCustomReferenceObject(FSoftObjectPath(Group.Materials[Index]));
	}
	FInterchangePipelineContextParams Context;
	Context.ContextType = EInterchangePipelineContext::AssetImport;
	Context.BaseNodeContainer = Container;
	Pipeline->AdjustSettingsForContext(Context);
	UInterchangeSourceData* Source = NewObject<UInterchangeSourceData>();
	Source->SetFilename(TEXT("Group.fbx"));
	Pipeline->ScriptedExecutePipeline(Container, {Source}, Group.Folder);
	for (int32 Index : {1, 0})
	{
		Pipeline->ScriptedExecutePostImportPipeline(Container, FactoryUids[Index], Group.Materials[Index], false);
		TestEqual(TEXT("The generic material rule assigns its selected parent"), Group.Materials[Index]->Parent.Get(), static_cast<UMaterialInterface*>(RuleParent));
		TestEqual(TEXT("Generic-rule MIs share the group's one local Toon Profile"), Group.Materials[Index]->ToonProfileOverride.Get(), Group.Shared);
		const UOmniImportMaterialUserData* Ownership = ProfileOwnership(Group.Materials[Index]);
		if (TestNotNull(TEXT("Generic-rule callbacks record shared ownership"), Ownership))
		{
			TestEqual(TEXT("Generic-rule ownership points to the applied shared object"), Ownership->OwnedToonProfile.Get(), Group.Shared);
			TestEqual(TEXT("Generic-rule ownership uses the same group key"), Ownership->SharedProfileGroupKey, Group.GroupKey);
		}
	}
	TestEqual(TEXT("The generic-rule parent's template is not edited"), Group.Template->Settings.DiffuseRampOffsetStrength, 0.25f);
	return !HasAnyErrors();
}

#endif
