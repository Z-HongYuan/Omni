// Copyright © 2026 张鸿源. All Rights Reserved.
#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Import/OmniCharacterImportPipeline.h"
#include "Import/OmniCharacterImportProfile.h"
#include "Import/Materials/OmniImportMaterialApplier.h"
#include "Import/Materials/OmniImportTextureSettings.h"
#include "Import/Materials/OmniImportToonProfile.h"
#include "Import/Assets/OmniImportAssetLayout.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Texture2D.h"
#include "Engine/ToonProfile.h"
#include "InterchangeMaterialFactoryNode.h"
#include "InterchangeMaterialInstanceNode.h"
#include "InterchangeSkeletalMeshFactoryNode.h"
#include "InterchangeStaticMeshFactoryNode.h"
#include "InterchangeSkeletonFactoryNode.h"
#include "InterchangePhysicsAssetFactoryNode.h"
#include "InterchangeAnimSequenceFactoryNode.h"
#include "InterchangeSourceData.h"
#include "InterchangeTextureFactoryNode.h"
#include "InterchangeTexture2DFactoryNode.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/Guid.h"
#include "Nodes/InterchangeBaseNodeContainer.h"
#include "UObject/Package.h"

namespace
{
constexpr EAutomationTestFlags TestFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

template <typename T>
T* AddNode(UInterchangeBaseNodeContainer* Container, const FString& Uid, const FString& Label, EInterchangeNodeContainerType Type)
{
	T* Node = NewObject<T>(Container);
	Node->InitializeNode(Uid, Label, Type);
	Container->AddNode(Node);
	return Node;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOmniImportLegacyValidationTest, "Omni.Editor.CharacterImport.ActiveModeValidation", TestFlags)
bool FOmniImportLegacyValidationTest::RunTest(const FString& Parameters)
{
	UOmniCharacterImportPipeline* Pipeline = NewObject<UOmniCharacterImportPipeline>();
	UOmniCharacterImportProfile* Profile = NewObject<UOmniCharacterImportProfile>();
	Profile->bConfigureMaterials = true;
	Profile->MaterialRules.AddDefaulted(); // 故意设置无效规则，验证关键词模式不使用该规则。
	Pipeline->Profile = Profile;
	Pipeline->bConfigureFaceMaterial = false;
	Pipeline->bConfigureBodyMaterial = false;
	TOptional<FText> Reason;
	TestTrue(TEXT("Unused legacy configuration cannot block keyword import"), Pipeline->IsSettingsAreValid(Reason));
	Pipeline->bUseKeywordMaterials = false;
	TestFalse(TEXT("The same invalid rule is rejected when active"), Pipeline->IsSettingsAreValid(Reason));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOmniImportProfileOwnershipTest, "Omni.Editor.CharacterImport.ProfileOwnership", TestFlags)
bool FOmniImportProfileOwnershipTest::RunTest(const FString& Parameters)
{
	UMaterialInstanceConstant* Material = NewObject<UMaterialInstanceConstant>();
	UToonProfile* Owned = NewObject<UToonProfile>();
	UToonProfile* Manual = NewObject<UToonProfile>();
	Omni::CharacterImport::SetOwnedToonProfile(Material, Owned);
	Omni::CharacterImport::ClearOwnedToonProfile(Material);
	TestFalse(TEXT("Leaving Body disables the owned override"), Material->bOverrideToonProfile != 0);
	TestNull(TEXT("Leaving Body clears the owned profile"), Material->ToonProfileOverride.Get());
	Omni::CharacterImport::SetOwnedToonProfile(Material, Owned);
	Material->ToonProfileOverride = Manual;
	Omni::CharacterImport::ClearOwnedToonProfile(Material);
	TestEqual(TEXT("A user replacement survives role changes"), Material->ToonProfileOverride.Get(), Manual);
	TestTrue(TEXT("A user replacement remains enabled"), Material->bOverrideToonProfile != 0);
	Omni::CharacterImport::ClearOwnedToonProfile(Material);
	TestEqual(TEXT("Unmarked assignments remain untouched"), Material->ToonProfileOverride.Get(), Manual);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOmniImportRetainedMaterialsTest, "Omni.Editor.CharacterImport.MeshOnlyMaterialUpdate", TestFlags)
bool FOmniImportRetainedMaterialsTest::RunTest(const FString& Parameters)
{
	UMaterialInterface* Parent = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/SharedContent/Material/ToonShader/Toon_BSDF/M_Toon_Face.M_Toon_Face"));
	if (!TestNotNull(TEXT("Read-only Face parent fixture"), Parent)) return false;
	UOmniCharacterImportPipeline* Pipeline = NewObject<UOmniCharacterImportPipeline>();
	UOmniCharacterImportProfile* Profile = NewObject<UOmniCharacterImportProfile>();
	Profile->bCreateIKRig = false;
	Profile->bCreatePostProcessAnimBlueprint = false;
	Profile->bCreateCharacterFolder = false;
	Profile->MaterialFolder = TEXT("Materials");
	Pipeline->Profile = Profile;
	Pipeline->CharacterName = TEXT("ReimportRegression");
	Pipeline->FaceParentMaterial = Parent;
	Pipeline->OutlineParentMaterial = Parent; // 本测试中两种材质类型都暴露 Tex_BaseColor。
	UInterchangeBaseNodeContainer* Container = NewObject<UInterchangeBaseNodeContainer>();
	USkeletalMesh* Mesh = NewObject<USkeletalMesh>();
	AddNode<UInterchangeSkeletalMeshFactoryNode>(Container, TEXT("Mesh"), TEXT("ReimportRegression"), EInterchangeNodeContainerType::FactoryData);
	UTexture2D* Before = NewObject<UTexture2D>();
	UTexture2D* After = NewObject<UTexture2D>();
	UToonProfile* OwnedProfile = NewObject<UToonProfile>();
	UToonProfile* UserProfile = NewObject<UToonProfile>();
	Pipeline->BodyToonProfileTemplate = OwnedProfile;
	const FString Destination = TEXT("/Game/OmniAutomation/RetainedMaterials_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
	UPackage* SharedPackage = CreatePackage(*(Destination / TEXT("Materials/TP_ReimportRegression")));
	SharedPackage->SetFlags(RF_Transient);
	UToonProfile* SharedProfile = NewObject<UToonProfile>(SharedPackage, TEXT("TP_ReimportRegression"), RF_Transient);
	TArray<UMaterialInstanceConstant*> Materials;
	for (const FString Label : {TEXT("Fixture_Face"), TEXT("Fixture_Face_Outline")})
	{
		AddNode<UInterchangeMaterialInstanceNode>(Container, Label, Label, EInterchangeNodeContainerType::TranslatedAsset);
		const FString FactoryUid = UInterchangeFactoryBaseNode::BuildFactoryNodeUid(Label);
		UInterchangeMaterialInstanceFactoryNode* Factory = AddNode<UInterchangeMaterialInstanceFactoryNode>(
			Container, FactoryUid, Label, EInterchangeNodeContainerType::FactoryData);
		UMaterialInstanceConstant* Material = NewObject<UMaterialInstanceConstant>();
		Material->SetParentEditorOnly(Parent);
		Omni::CharacterImport::SetOwnedToonProfile(Material, OwnedProfile);
		TestTrue(TEXT("Native setter verification succeeds"), Omni::CharacterImport::SetTextureParameterVerified(Material, TEXT("Tex_BaseColor"), Before));
		Factory->SetCustomReferenceObject(FSoftObjectPath(Material));
		Mesh->GetMaterials().Add(FSkeletalMaterial(Material));
		Materials.Add(Material);
		FOmniKeywordMaterialBinding& Section = Pipeline->MaterialMappings.AddDefaulted_GetRef();
		Section.SourceMaterial = Label;
		FOmniFaceTextureBinding& Binding = Section.Textures.AddDefaulted_GetRef();
		Binding.Parameter = TEXT("Tex_BaseColor");
		Binding.Mode = EOmniFaceTextureMode::ExistingTexture;
		Binding.Texture = After;
	}
	Materials[1]->ToonProfileOverride = UserProfile;
	FInterchangePipelineContextParams Context;
	Context.ContextType = EInterchangePipelineContext::AssetReimport;
	Context.ReimportAsset = Mesh;
	Context.BaseNodeContainer = Container;
	Pipeline->AdjustSettingsForContext(Context);
	UInterchangeSourceData* Source = NewObject<UInterchangeSourceData>();
	Source->SetFilename(TEXT("ReimportRegression.fbx"));
	auto RunMeshCallback = [&]()
	{
		Pipeline->ScriptedExecutePipeline(Container, {Source}, Destination);
		// 只触发网格体回调，验证没有材质回调时的重导入路径。
		Pipeline->ScriptedExecutePostImportPipeline(Container, TEXT("Mesh"), Mesh, true);
	};
	RunMeshCallback();
	TestEqual(TEXT("Ordinary reimport preserves the prior Body override"), Materials[0]->ToonProfileOverride.Get(), OwnedProfile);
	for (UMaterialInstanceConstant* Material : Materials)
	{
		UTexture* Actual = nullptr;
		Material->GetTextureParameterValue(FMaterialParameterInfo(TEXT("Tex_BaseColor")), Actual);
		TestEqual(TEXT("Ordinary mesh reimport preserves manual overrides"), Actual, static_cast<UTexture*>(Before));
	}
	Pipeline->bUpdateKeywordMaterialsOnReimport = true;
	RunMeshCallback();
	TestEqual(TEXT("Explicit mesh-only update migrates a pipeline-owned override to the FBX group"), Materials[0]->ToonProfileOverride.Get(), SharedProfile);
	TestEqual(TEXT("Body to Outline preserves a user replacement"), Materials[1]->ToonProfileOverride.Get(), UserProfile);
	for (UMaterialInstanceConstant* Material : Materials)
	{
		UTexture* Actual = nullptr;
		Material->GetTextureParameterValue(FMaterialParameterInfo(TEXT("Tex_BaseColor")), Actual);
		TestEqual(TEXT("Explicit mesh-only update reaches both Face and Outline"), Actual, static_cast<UTexture*>(After));
	}
	SharedPackage->SetDirtyFlag(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOmniImportTextureSettingsTest, "Omni.Editor.CharacterImport.TextureUsage", TestFlags)
bool FOmniImportTextureSettingsTest::RunTest(const FString& Parameters)
{
	UInterchangeBaseNodeContainer* Container = NewObject<UInterchangeBaseNodeContainer>();
	TArray<FOmniKeywordMaterialBinding> Rows;
	FOmniKeywordMaterialBinding& Row = Rows.AddDefaulted_GetRef();
	for (const FName Parameter : {FName(TEXT("Tex_BaseColor")), FName(TEXT("Tex_SDF")), FName(TEXT("Tex_Normal")), FName(TEXT("Tex_Ramp")),
		FName(TEXT("Tex_LightMap")), FName(TEXT("Tex_Shadow")), FName(TEXT("Tex_MetalMap"))})
	{
		const FString Uid = Parameter.ToString();
		UInterchangeTextureFactoryNode* Node = AddNode<UInterchangeTexture2DFactoryNode>(Container,
			UInterchangeFactoryBaseNode::BuildFactoryNodeUid(Uid), Uid, EInterchangeNodeContainerType::FactoryData);
		Node->SetCustomSRGB(true);
		Node->SetCustomCompressionSettings(TC_Default);
		FOmniFaceTextureBinding& Binding = Row.Textures.AddDefaulted_GetRef();
		Binding.Parameter = Parameter;
		Binding.Mode = EOmniFaceTextureMode::AutoMatch;
		Binding.ResolvedTextureUid = Uid;
	}
	int32 Errors = 0;
	const Omni::CharacterImport::FImportReporter Reporter{[&Errors](const FString&, bool bError) { Errors += bError ? 1 : 0; }};
	TestTrue(TEXT("Compatible uses are accepted"), Omni::CharacterImport::PrepareTextureSettings(Container, Rows, Reporter));
	for (const FOmniFaceTextureBinding& Binding : Row.Textures)
	{
		const UInterchangeTextureFactoryNode* Node = CastChecked<UInterchangeTextureFactoryNode>(Container->GetFactoryNode(
			UInterchangeFactoryBaseNode::BuildFactoryNodeUid(Binding.ResolvedTextureUid)));
		bool bSRGB = false;
		uint8 Compression = TC_Default;
		Node->GetCustomSRGB(bSRGB);
		Node->GetCustomCompressionSettings(Compression);
		const bool bData = Binding.Parameter == TEXT("Tex_SDF") || Binding.Parameter == TEXT("Tex_LightMap") ||
			Binding.Parameter == TEXT("Tex_Shadow") || Binding.Parameter == TEXT("Tex_MetalMap");
		const bool bNormal = Binding.Parameter == TEXT("Tex_Normal");
		TestEqual(TEXT("Color/Ramp retain sRGB; data and normal are linear"), bSRGB, !bData && !bNormal);
		TestEqual(TEXT("Mask data uses Default compression; normals retain Normalmap"), Compression, static_cast<uint8>(bNormal ? TC_Normalmap : TC_Default));
	}
	Row.Textures[1].ResolvedTextureUid = Row.Textures[0].ResolvedTextureUid;
	TestFalse(TEXT("One source cannot be both color and data"), Omni::CharacterImport::PrepareTextureSettings(Container, Rows, Reporter));
	TestEqual(TEXT("Conflict is reported once"), Errors, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOmniImportMultiAssetLayoutTest, "Omni.Editor.CharacterImport.MultiAssetLayout", TestFlags)
bool FOmniImportMultiAssetLayoutTest::RunTest(const FString& Parameters)
{
	UInterchangeBaseNodeContainer* Container = NewObject<UInterchangeBaseNodeContainer>();
	UOmniCharacterImportProfile* Profile = NewObject<UOmniCharacterImportProfile>();
	Profile->MeshFolder = TEXT("Meshes");
	UInterchangeSkeletalMeshFactoryNode* Body = AddNode<UInterchangeSkeletalMeshFactoryNode>(Container, TEXT("BodyMesh"), TEXT("Body"), EInterchangeNodeContainerType::FactoryData);
	UInterchangeSkeletalMeshFactoryNode* Coat = AddNode<UInterchangeSkeletalMeshFactoryNode>(Container, TEXT("CoatMesh"), TEXT("Coat"), EInterchangeNodeContainerType::FactoryData);
	UInterchangeSkeletalMeshFactoryNode* Weapon = AddNode<UInterchangeSkeletalMeshFactoryNode>(Container, TEXT("WeaponMesh"), TEXT("Weapon"), EInterchangeNodeContainerType::FactoryData);
	UInterchangeStaticMeshFactoryNode* Scope = AddNode<UInterchangeStaticMeshFactoryNode>(Container, TEXT("ScopeMesh"), TEXT("Scope"), EInterchangeNodeContainerType::FactoryData);
	UInterchangeSkeletonFactoryNode* HumanSkeleton = AddNode<UInterchangeSkeletonFactoryNode>(Container, TEXT("HumanSkeleton"), TEXT("HumanRoot"), EInterchangeNodeContainerType::FactoryData);
	HumanSkeleton->AddCustomSkeletalMeshFactoryNodeUid(Body->GetUniqueID());
	HumanSkeleton->AddCustomSkeletalMeshFactoryNodeUid(Coat->GetUniqueID());
	UInterchangeSkeletonFactoryNode* WeaponSkeleton = AddNode<UInterchangeSkeletonFactoryNode>(Container, TEXT("WeaponSkeleton"), TEXT("WeaponRoot"), EInterchangeNodeContainerType::FactoryData);
	WeaponSkeleton->AddCustomSkeletalMeshFactoryNodeUid(Weapon->GetUniqueID());
	UInterchangePhysicsAssetFactoryNode* Physics = AddNode<UInterchangePhysicsAssetFactoryNode>(Container, TEXT("WeaponPhysics"), TEXT("Physics"), EInterchangeNodeContainerType::FactoryData);
	Physics->SetCustomSkeletalMeshUid(Weapon->GetUniqueID());
	UInterchangeAnimSequenceFactoryNode* Animation = AddNode<UInterchangeAnimSequenceFactoryNode>(Container, TEXT("FireAnimation"), TEXT("Weapon_Fire"), EInterchangeNodeContainerType::FactoryData);
	Animation->SetCustomSkeletonFactoryNodeUid(WeaponSkeleton->GetUniqueID());
	UInterchangeAnimSequenceFactoryNode* OtherAnimation = AddNode<UInterchangeAnimSequenceFactoryNode>(Container, TEXT("OtherFireAnimation"), TEXT("Weapon_Fire"), EInterchangeNodeContainerType::FactoryData);
	OtherAnimation->SetCustomSkeletonFactoryNodeUid(HumanSkeleton->GetUniqueID());
	UInterchangeSkeletalMeshFactoryNode* PrefixedBody = AddNode<UInterchangeSkeletalMeshFactoryNode>(Container, TEXT("PrefixedBodyMesh"), TEXT("SK_Body"), EInterchangeNodeContainerType::FactoryData);
	UInterchangeSkeletonFactoryNode* PrefixedSkeleton = AddNode<UInterchangeSkeletonFactoryNode>(Container, TEXT("PrefixedSkeleton"), TEXT("OtherRoot"), EInterchangeNodeContainerType::FactoryData);
	PrefixedSkeleton->AddCustomSkeletalMeshFactoryNodeUid(PrefixedBody->GetUniqueID());
	UInterchangeTexture2DFactoryNode* Texture = AddNode<UInterchangeTexture2DFactoryNode>(Container, TEXT("SourceTexture"), TEXT("Original_Hair_ILM"), EInterchangeNodeContainerType::FactoryData);
	Omni::CharacterImport::ApplyAssetLayout(Container, *Profile, TEXT("Assembly"), false);
	TestEqual(TEXT("Body keeps its own asset name"), Body->GetAssetName(), FString(TEXT("SKM_Body")));
	TestEqual(TEXT("Coat stays independent despite sharing a skeleton"), Coat->GetAssetName(), FString(TEXT("SKM_Coat")));
	TestEqual(TEXT("Weapon keeps its own asset name"), Weapon->GetAssetName(), FString(TEXT("SKM_Weapon")));
	TestEqual(TEXT("Static attachment gets its own name"), Scope->GetAssetName(), FString(TEXT("SM_Scope")));
	TestEqual(TEXT("Human skeleton follows an owning mesh"), HumanSkeleton->GetAssetName(), FString(TEXT("SK_Body")));
	TestEqual(TEXT("Weapon skeleton is distinct"), WeaponSkeleton->GetAssetName(), FString(TEXT("SK_Weapon")));
	TestEqual(TEXT("Physics follows its actual owning mesh"), Physics->GetAssetName(), FString(TEXT("PHYS_Weapon")));
	TestEqual(TEXT("Texture source name is unchanged"), Texture->GetAssetName(), FString(TEXT("Original_Hair_ILM")));
	TestEqual(TEXT("Animation source name is unchanged"), Animation->GetAssetName(), FString(TEXT("Weapon_Fire")));
	TestNotEqual(TEXT("Same-named animations for different skeletons cannot overwrite each other"), OtherAnimation->GetAssetName(), Animation->GetAssetName());
	TestNotEqual(TEXT("Existing source prefixes cannot collapse distinct skeletons"), PrefixedSkeleton->GetAssetName(), HumanSkeleton->GetAssetName());
	FString SubPath;
	HumanSkeleton->GetCustomSubPath(SubPath);
	TestEqual(TEXT("Shared skeleton remains beside meshes in a custom MeshFolder"), SubPath, FString(TEXT("Assembly/Meshes")));
	Animation->GetCustomSubPath(SubPath);
	TestEqual(TEXT("Animation is organized in the import group"), SubPath, FString(TEXT("Assembly/Animations")));
	TArray<FString> SharedMeshes;
	HumanSkeleton->GetCustomSkeletalMeshFactoryNodeUids(SharedMeshes);
	TestEqual(TEXT("Shared skeleton still refers to both meshes"), SharedMeshes.Num(), 2);
	FString SkeletonUid;
	Animation->GetCustomSkeletonFactoryNodeUid(SkeletonUid);
	TestEqual(TEXT("Animation still targets the weapon skeleton"), SkeletonUid, WeaponSkeleton->GetUniqueID());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOmniImportMixedFactoriesTest, "Omni.Editor.CharacterImport.MixedMeshFactories", TestFlags)
bool FOmniImportMixedFactoriesTest::RunTest(const FString& Parameters)
{
	for (const bool bStaticOnly : {false, true})
	{
		UOmniCharacterImportPipeline* Pipeline = NewObject<UOmniCharacterImportPipeline>();
		UOmniCharacterImportProfile* Profile = NewObject<UOmniCharacterImportProfile>();
		Profile->bCreateIKRig = false;
		Profile->bCreatePostProcessAnimBlueprint = false;
		Pipeline->Profile = Profile;
		Pipeline->bUseKeywordMaterials = false;
		Pipeline->bConfigureFaceMaterial = false;
		Pipeline->bConfigureBodyMaterial = false;
		UInterchangeBaseNodeContainer* Container = NewObject<UInterchangeBaseNodeContainer>();
		UInterchangeStaticMeshFactoryNode* Static = AddNode<UInterchangeStaticMeshFactoryNode>(Container, TEXT("Static"), TEXT("Attachment"), EInterchangeNodeContainerType::FactoryData);
		if (!bStaticOnly)
		{
			AddNode<UInterchangeSkeletalMeshFactoryNode>(Container, TEXT("Body"), TEXT("Body"), EInterchangeNodeContainerType::FactoryData);
			AddNode<UInterchangeSkeletalMeshFactoryNode>(Container, TEXT("Weapon"), TEXT("Weapon"), EInterchangeNodeContainerType::FactoryData);
		}
		UInterchangeSourceData* Source = NewObject<UInterchangeSourceData>();
		Source->SetFilename(TEXT("Assembly.fbx"));
		Pipeline->ScriptedExecutePipeline(Container, {Source}, TEXT("/Game/ImportTests/MixedFactories"));
		TestTrue(TEXT("Static-only and mixed multi-mesh imports stay enabled"), Static->IsEnabled());
		TestTrue(TEXT("Static assets receive the Omni layout"), Static->GetAssetName().StartsWith(TEXT("SM_")));
	}
	return true;
}

#endif
