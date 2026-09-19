// Copyright © 2026 张鸿源. All Rights Reserved.
#include "Import/OmniCharacterImportPipeline.h"
#include "Import/Materials/OmniImportMaterialResolver.h"
#include "Import/Materials/OmniImportMaterialApplier.h"
#include "InterchangeMaterialFactoryNode.h"
#include "InterchangeMaterialInstanceNode.h"
#include "InterchangeShaderGraphNode.h"
#include "Materials/MaterialInterface.h"
#include "Nodes/InterchangeBaseNodeContainer.h"

namespace
{
	bool IsBodySource(FString Name)
	{
		Name.ReplaceInline(TEXT(" "), TEXT("_"));
		Name.ReplaceInline(TEXT("."), TEXT("_"));
		Name.ReplaceInline(TEXT("-"), TEXT("_"));
		TArray<FString> Words;
		Name.ParseIntoArray(Words, TEXT("_"), true);
		return Words.ContainsByPredicate([](const FString& Word) { return Word.Equals(TEXT("Body"), ESearchCase::IgnoreCase); }) &&
			!Name.Contains(TEXT("Outline")) && !Name.Contains(TEXT("Face")) && !Name.Contains(TEXT("Hair"));
	}
}


void UOmniCharacterImportPipeline::RefreshBodyTextureParameters()
{
	UMaterialInterface* Parent = BodyParentMaterial.LoadSynchronous();
	for (FOmniBodyMaterialBinding& Section : BodyMaterials) Omni::CharacterImport::RefreshMaterialTextureParameters(Parent, Section.Textures);
}

void UOmniCharacterImportPipeline::RefreshBodySourceData(UInterchangeBaseNodeContainer* Container)
{
	BodyValidationError.Reset();
	if (!bConfigureBodyMaterial)
	{
		BodyStatus = TEXT("已关闭 Body 材质配置。");
		return;
	}
	if (!Container)
	{
		RefreshBodyTextureParameters();
		BodyStatus = TEXT("导入 FBX 后按源 Body 材质逐项配置；描边不参与自动识别。");
		return;
	}
	TArray<UInterchangeBaseNode*> Materials;
	Container->GetNodesOfType(Materials);
	Materials = Materials.FilterByPredicate([](const UInterchangeBaseNode* Node)
	{
		return Node->IsA<UInterchangeShaderGraphNode>() || Node->IsA<UInterchangeMaterialInstanceNode>();
	});
	Materials.Sort([](const UInterchangeBaseNode& A, const UInterchangeBaseNode& B) { return A.GetDisplayLabel() < B.GetDisplayLabel(); });
	// Preserve per-section choices across dialog rebuilds and reimports.
	if (BodyMaterials.IsEmpty())
	{
		for (const UInterchangeBaseNode* Node : Materials)
		{
			if (!IsBodySource(Node->GetDisplayLabel())) continue;
			FOmniBodyMaterialBinding& Section = BodyMaterials.AddDefaulted_GetRef();
			Section.SourceMaterial = Node->GetDisplayLabel();
		}
	}
	RefreshBodyTextureParameters();
	TSet<FString> UsedSources;
	int32 ActiveCount = 0;
	for (FOmniBodyMaterialBinding& Section : BodyMaterials)
	{
		Section.ResolvedMaterialUid.Reset();
		if (!Section.bEnabled)
		{
			Section.Status = TEXT("跳过此材质。");
			continue;
		}
		++ActiveCount;
		const TArray<UInterchangeBaseNode*> Matches = Materials.FilterByPredicate([&](const UInterchangeBaseNode* Node)
		{
			return Node->GetDisplayLabel() == Section.SourceMaterial;
		});
		if (Matches.Num() != 1 || UsedSources.Contains(Section.SourceMaterial))
		{
			Section.Status = TEXT("请选择唯一且不重复的源 Body 材质，或关闭此项。");
			BodyValidationError = Section.SourceMaterial + TEXT("：") + Section.Status;
			continue;
		}
		UsedSources.Add(Section.SourceMaterial);
		Section.ResolvedMaterialUid = Matches[0]->GetUniqueID();
		if (bConfigureFaceMaterial && Section.ResolvedMaterialUid == FaceSourceUid)
		{
			Section.Status = TEXT("同一源材质不能同时配置为 Face 和 Body。");
			BodyValidationError = Section.Status;
			continue;
		}
		if (Section.SourceMaterial.Contains(TEXT("Outline")))
		{
			Section.Status = TEXT("Body 配置不能用于描边材质。");
			BodyValidationError = Section.Status;
			continue;
		}
		FString Error;
		Omni::CharacterImport::ResolveMaterialTextures(Container, Matches[0], TEXT("Body"), Section.Textures, Error);
		Section.Status = Error.IsEmpty() ? TEXT("按此源材质独立绑定纹理；未找到的自动纹理沿用默认值。") : Error;
		if (!Error.IsEmpty()) BodyValidationError = Section.SourceMaterial + TEXT("：") + Error;
	}
	if (ActiveCount > 0 && !BodyParentMaterial.LoadSynchronous()) BodyValidationError = TEXT("请选择有效的 Body 母材质。");
	else if (ActiveCount > 0 && !ResolveBodyToonProfileTemplate())
		BodyValidationError = TEXT("Body 母材质没有唯一的 Toon Profile，请选择 Body Profile 模板。");
	BodyStatus = BodyValidationError.IsEmpty()
		             ? FString::Printf(TEXT("配置 %d 个 Body 材质分区；每个模型复制一个独立 Toon Profile，仅覆盖 Body。"), ActiveCount)
		             : BodyValidationError;
	if (bIsReimportContext && !bUpdateBodyMaterialOnReimport)
		BodyStatus = TEXT("重导入保留现有 Body 纹理和 Profile；勾选更新后才重新应用配置。");
}

bool UOmniCharacterImportPipeline::ValidateBodySettings(TOptional<FText>& OutInvalidReason) const
{
	if (!bConfigureBodyMaterial || (bIsReimportContext && !bUpdateBodyMaterialOnReimport)) return true;
	if (!BodyValidationError.IsEmpty())
	{
		OutInvalidReason = FText::FromString(BodyValidationError);
		return false;
	}
	return true;
}

bool UOmniCharacterImportPipeline::PrepareBodyMaterials(UInterchangeBaseNodeContainer* Container)
{
	if (!bConfigureBodyMaterial) return true;
	for (int32 Index = 0; Index < BodyMaterials.Num(); ++Index)
	{
		const FOmniBodyMaterialBinding& Section = BodyMaterials[Index];
		if (!Section.bEnabled || Section.ResolvedMaterialUid.IsEmpty()) continue;
		const FString Key = UInterchangeFactoryBaseNode::BuildFactoryNodeUid(Section.ResolvedMaterialUid);
		BodyFactoryRows.Add(Key, Index);
		if (bIsReimportContext && !bUpdateBodyMaterialOnReimport) continue;
		if (!Cast<UInterchangeMaterialInstanceFactoryNode>(Container->GetFactoryNode(Key)))
		{
			Report(TEXT("Body needs Material Instances enabled in the preceding FBX pipeline: ") + Section.SourceMaterial, true);
			return false;
		}
		if (!Omni::CharacterImport::PrepareMaterialTextures(Container, Key, BodyParentMaterial, Section.Textures, {[this](const FString& Message, bool bError) { Report(Message, bError); }})) return false;
	}
	return true;
}

void UOmniCharacterImportPipeline::ApplyBodyMaterial(const UInterchangeBaseNodeContainer* Container,
                                                     const FString& NodeKey, UMaterialInstanceConstant* Material, const bool bReimport)
{
	if ((bReimport || bIsReimportContext) && !bUpdateBodyMaterialOnReimport) return;
	const int32* Index = BodyFactoryRows.Find(NodeKey);
	if (!Index || !BodyMaterials.IsValidIndex(*Index)) return;
	Omni::CharacterImport::ApplyMaterialTextures(Container, NodeKey, Material, BodyParentMaterial.LoadSynchronous(), BodyMaterials[*Index].Textures, {[this](const FString& Message, bool bError) { Report(Message, bError); }});
}
