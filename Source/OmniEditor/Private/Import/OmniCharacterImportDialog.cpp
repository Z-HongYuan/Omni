// Copyright © 2026 张鸿源. All Rights Reserved.
#include "Import/OmniCharacterImportPipeline.h"
#include "Import/Materials/OmniImportMaterialResolver.h"
#include "InterchangeTextureNode.h"
#include "Nodes/InterchangeBaseNodeContainer.h"

UOmniCharacterImportPipeline::UOmniCharacterImportPipeline()
{
	OutlineParentMaterial = FSoftObjectPath(TEXT("/SharedContent/Material/ToonShader/Toon_BSDF/M_Toon_Outline.M_Toon_Outline"));
	for (const auto& Entry : TArray<FOmniTextureKeywordRule>{
		{TEXT("Tex_BaseColor"), {TEXT("BaseColor"), TEXT("Diffuse"), TEXT("Albedo")}},
		{TEXT("Tex_LightMap"), {TEXT("LightMap"), TEXT("ILM")}},
		{TEXT("Tex_Normal"), {TEXT("Normal"), TEXT("NormalMap")}},
		{TEXT("Tex_Ramp"), {TEXT("Ramp"), TEXT("ShadowRamp"), TEXT("SpecularRamp")}},
		{TEXT("Tex_SDF"), {TEXT("SDF"), TEXT("FaceLightMap")}},
		{TEXT("Tex_Shadow"), {TEXT("Shadow")}},
		{TEXT("Tex_MetalMap"), {TEXT("MetalMap"), TEXT("Metallic"), TEXT("Metalness")}}}) TextureKeywords.Add(Entry);
	FaceParentMaterial = FSoftObjectPath(TEXT("/SharedContent/Material/ToonShader/Toon_BSDF/M_Toon_Face.M_Toon_Face"));
	BodyParentMaterial = FSoftObjectPath(TEXT("/SharedContent/Material/ToonShader/Toon_BSDF/M_Toon_Body.M_Toon_Body"));
	for (const FName Name : {
		     GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, bUseKeywordMaterials),
		     GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, OutlineParentMaterial),
		     GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, MaterialMappings),
		     GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, TextureKeywords),
		     GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, MaterialMappingStatus),
		     GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, bUpdateKeywordMaterialsOnReimport),
		     GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, bConfigureFaceMaterial),
		     GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, FaceParentMaterial),
		     GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, FaceSourceMaterial),
		     GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, FaceTextures),
		     GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, FaceStatus),
		     GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, bUpdateFaceMaterialOnReimport),
		     GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, bConfigureBodyMaterial),
		     GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, BodyParentMaterial),
		     GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, BodyToonProfileTemplate),
		     GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, BodyMaterials),
		     GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, BodyStatus),
		     GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, bUpdateBodyMaterialOnReimport),
		     GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, bNormalizeTextureNames),
		     GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, TextureNames),
		     GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, TextureNamingStatus)
	     })
	{
		const FProperty* Property = FindFProperty<FProperty>(StaticClass(), Name);
		FindOrAddPropertyStates(FName(Property->GetPathName())).SetPropertyShowEssentialsVisibility(true);
	}
}

void UOmniCharacterImportPipeline::AdjustSettingsForContext(const FInterchangePipelineContextParams& ContextParams)
{
	Super::AdjustSettingsForContext(ContextParams);
	// Updating a material on reimport is an explicit per-run decision.
	bUpdateFaceMaterialOnReimport = false;
	bUpdateBodyMaterialOnReimport = false;
	bUpdateKeywordMaterialsOnReimport = false;
	// Populate options even when the dialog's Filter Options toggle is off.
	RefreshMaterialSourceData(const_cast<UInterchangeBaseNodeContainer*>(ContextParams.BaseNodeContainer.Get()));
	RefreshTextureNames(const_cast<UInterchangeBaseNodeContainer*>(ContextParams.BaseNodeContainer.Get()));
}

void UOmniCharacterImportPipeline::FilterPropertiesFromTranslatedData(UInterchangeBaseNodeContainer* Container)
{
	Super::FilterPropertiesFromTranslatedData(Container);
	RefreshMaterialSourceData(Container);
	RefreshTextureNames(Container);
}

void UOmniCharacterImportPipeline::PreDialogCleanup(FName PipelineStackName)
{
	// This hook runs for a NEW import dialog, not for refreshes or asset reimports.
	// AlwaysResetToDefault metadata would also discard edits on every UI refresh.
	FaceSourceMaterial.Reset();
	FaceTextures.Reset();
	BodyMaterials.Reset();
	TextureNames.Reset();
	MaterialMappings.Reset();
	KeywordMaterialError.Reset();
	TextureNamingError.Reset();
	bUpdateBodyMaterialOnReimport = false;
	bUpdateKeywordMaterialsOnReimport = false;
	bUpdateFaceMaterialOnReimport = false;
	RefreshFaceTextureParameters();
	SaveSettings(PipelineStackName);
}

bool UOmniCharacterImportPipeline::IsPropertyChangeNeedRefresh(const FPropertyChangedEvent& Event) const
{
	const FName Name = Event.GetMemberPropertyName();
	return Name == GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, bUseKeywordMaterials) ||
		Name == GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, MaterialMappings) ||
		Name == GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, TextureKeywords) ||
		Name == GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, OutlineParentMaterial) ||
		Name == GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, bUpdateKeywordMaterialsOnReimport) ||
		Name == GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, FaceParentMaterial) ||
		Name == GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, FaceTextures) ||
		Name == GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, FaceSourceMaterial) ||
		Name == GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, bConfigureFaceMaterial) ||
		Name == GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, bUpdateFaceMaterialOnReimport) ||
		Name == GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, BodyParentMaterial) ||
		Name == GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, BodyToonProfileTemplate) ||
		Name == GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, BodyMaterials) ||
		Name == GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, bConfigureBodyMaterial) ||
		Name == GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, bUpdateBodyMaterialOnReimport) ||
		Name == GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, bNormalizeTextureNames) ||
		Name == GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, TextureNames) ||
		Name == GET_MEMBER_NAME_CHECKED(UOmniCharacterImportPipeline, CharacterName);
}

TArray<FString> UOmniCharacterImportPipeline::GetFaceSourceMaterialOptions() const
{
	TArray<FString> Values;
	Values.Add(FString());
	if (FaceSourceContainer)
	{
		for (const UInterchangeBaseNode* Node : Omni::CharacterImport::GetSourceMaterials(FaceSourceContainer)) Values.AddUnique(Node->GetDisplayLabel());
	}
	Values.Sort();
	return Values;
}

TArray<FString> UOmniCharacterImportPipeline::GetFaceSourceTextureOptions() const
{
	TArray<FString> Values;
	Values.Add(FString());
	if (FaceSourceContainer)
	{
		TArray<UInterchangeTextureNode*> Nodes;
		FaceSourceContainer->GetNodesOfType(Nodes);
		for (const UInterchangeTextureNode* Node : Nodes) Values.AddUnique(Node->GetDisplayLabel());
	}
	Values.Sort();
	return Values;
}
