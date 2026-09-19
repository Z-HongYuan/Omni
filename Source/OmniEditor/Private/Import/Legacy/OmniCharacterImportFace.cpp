// Copyright © 2026 张鸿源. All Rights Reserved.
#include "Import/OmniCharacterImportPipeline.h"
#include "Import/Materials/OmniImportMaterialResolver.h"
#include "Import/Materials/OmniImportMaterialApplier.h"
#include "InterchangeMaterialFactoryNode.h"
#include "Materials/MaterialInterface.h"
#include "Nodes/InterchangeBaseNodeContainer.h"

namespace
{
	bool IsFaceName(FString Name)
	{
		Name.ReplaceInline(TEXT(" "), TEXT("_"));
		Name.ReplaceInline(TEXT("."), TEXT("_"));
		Name.ReplaceInline(TEXT("-"), TEXT("_"));
		TArray<FString> Words;
		Name.ParseIntoArray(Words, TEXT("_"), true);
		const bool bFace = Words.ContainsByPredicate([](const FString& Word) { return Word.Equals(TEXT("Face"), ESearchCase::IgnoreCase); });
		return bFace && !Name.Contains(TEXT("Outline")) && !Name.Contains(TEXT("Eye")) &&
			!Name.Contains(TEXT("Brow")) && !Name.Contains(TEXT("Mouth"));
	}

}

void UOmniCharacterImportPipeline::RefreshFaceTextureParameters()
{
	Omni::CharacterImport::RefreshMaterialTextureParameters(FaceParentMaterial.LoadSynchronous(), FaceTextures);
}

void UOmniCharacterImportPipeline::RefreshFaceSourceData(UInterchangeBaseNodeContainer* Container)
{
	FaceSourceContainer = Container;
	FaceSourceUid.Reset();
	FaceValidationError.Reset();
	if (!bConfigureFaceMaterial)
	{
		FaceStatus = TEXT("已关闭 Face 材质配置。");
		return;
	}
	RefreshFaceTextureParameters();
	if (!FaceParentMaterial.LoadSynchronous())
	{
		FaceValidationError = TEXT("请选择有效的 Face 母材质。");
		FaceStatus = FaceValidationError;
		return;
	}
	if (!Container)
	{
		FaceStatus = TEXT("导入 FBX 后显示源材质、纹理选项与匹配结果。");
		return;
	}
	TArray<UInterchangeBaseNode*> Materials = Omni::CharacterImport::GetSourceMaterials(Container);
	Materials = Materials.FilterByPredicate([this](const UInterchangeBaseNode* Node)
	{
		return FaceSourceMaterial.IsEmpty() ? IsFaceName(Node->GetDisplayLabel()) : Node->GetDisplayLabel() == FaceSourceMaterial;
	});
	if (Materials.IsEmpty() && FaceSourceMaterial.IsEmpty())
	{
		FaceStatus = TEXT("未识别到 Face 材质，本次跳过；也可手动选择源 Face 材质。");
		return;
	}
	if (Materials.Num() != 1)
	{
		FaceValidationError = FString::Printf(TEXT("Face 源材质匹配到 %d 项，请选择唯一的源 Face 材质，或关闭 Face 配置。"), Materials.Num());
		FaceStatus = FaceValidationError;
		return;
	}
	FaceSourceUid = Materials[0]->GetUniqueID();
	Omni::CharacterImport::ResolveMaterialTextures(Container, Materials[0], TEXT("Face"), FaceTextures, FaceValidationError);
	FaceStatus = FaceValidationError.IsEmpty() ? TEXT("Face：") + Materials[0]->GetDisplayLabel() : FaceValidationError;
	if (bIsReimportContext && !bUpdateFaceMaterialOnReimport) FaceStatus = TEXT("重导入将保留现有 Face 材质；勾选更新后才应用这里的设置。");
}

bool UOmniCharacterImportPipeline::ValidateFaceSettings(TOptional<FText>& OutInvalidReason) const
{
	if (!bConfigureFaceMaterial || (bIsReimportContext && !bUpdateFaceMaterialOnReimport)) return true;
	if (!FaceValidationError.IsEmpty())
	{
		OutInvalidReason = FText::FromString(FaceValidationError);
		return false;
	}
	if (!FaceParentMaterial.LoadSynchronous())
	{
		OutInvalidReason = FText::FromString(TEXT("请选择有效的 Face 母材质。"));
		return false;
	}
	return true;
}

bool UOmniCharacterImportPipeline::PrepareFaceMaterial(UInterchangeBaseNodeContainer* Container)
{
	if (!bConfigureFaceMaterial || FaceSourceUid.IsEmpty()) return true;
	const FString FactoryUid = UInterchangeFactoryBaseNode::BuildFactoryNodeUid(FaceSourceUid);
	UInterchangeMaterialInstanceFactoryNode* Factory = Cast<UInterchangeMaterialInstanceFactoryNode>(Container->GetFactoryNode(FactoryUid));
	if (!Factory)
	{
		if (bIsReimportContext && !bUpdateFaceMaterialOnReimport) return true;
		Report(TEXT("Face needs Import Materials enabled and New Material Asset Type set to Material Instances in the preceding FBX pipeline."), true);
		return false;
	}
	FaceFactoryKeys.Add(FactoryUid); // Also protects Face from generic profile rules on reimport.
	if (bIsReimportContext && !bUpdateFaceMaterialOnReimport) return true;
	return Omni::CharacterImport::PrepareMaterialTextures(Container, FactoryUid, FaceParentMaterial, FaceTextures, {[this](const FString& Message, bool bError) { Report(Message, bError); }});
}

void UOmniCharacterImportPipeline::ApplyFaceMaterial(const UInterchangeBaseNodeContainer* Container, const FString& NodeKey, UMaterialInstanceConstant* Material, bool bReimport)
{
	if ((bReimport || bIsReimportContext) && !bUpdateFaceMaterialOnReimport)
	{
		Report(TEXT("Retained existing Face instance and texture overrides on reimport."));
		return;
	}
	Omni::CharacterImport::ApplyMaterialTextures(Container, NodeKey, Material, FaceParentMaterial.LoadSynchronous(), FaceTextures, {[this](const FString& Message, bool bError) { Report(Message, bError); }});
}
