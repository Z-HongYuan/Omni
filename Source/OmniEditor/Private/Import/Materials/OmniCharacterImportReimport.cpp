// Copyright © 2026 张鸿源. All Rights Reserved.
#include "Import/OmniCharacterImportPipeline.h"

#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Nodes/InterchangeBaseNodeContainer.h"
#include "Nodes/InterchangeFactoryBaseNode.h"

void UOmniCharacterImportPipeline::ApplyRetainedMaterials(const UInterchangeBaseNodeContainer* Container, UObject* Mesh)
{
	if (!Container || !Mesh) return;
	TSet<UMaterialInterface*> UsedMaterials;
	if (const USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(Mesh))
	{
		for (const FSkeletalMaterial& Slot : SkeletalMesh->GetMaterials()) UsedMaterials.Add(Slot.MaterialInterface);
	}
	else if (const UStaticMesh* StaticMesh = Cast<UStaticMesh>(Mesh))
	{
		for (const FStaticMaterial& Slot : StaticMesh->GetStaticMaterials()) UsedMaterials.Add(Slot.MaterialInterface);
	}
	// 仅重导入网格体时，保留的材质可能不触发导入后回调。
	// 只更新当前网格体实际使用的工厂引用，不按名称搜索其他材质。
	auto FindMaterial = [Container, &UsedMaterials](const FString& Key) -> UMaterialInstanceConstant*
	{
		const UInterchangeFactoryBaseNode* Node = Container->GetFactoryNode(Key);
		FSoftObjectPath Path;
		if (!Node || !Node->GetCustomReferenceObject(Path)) return nullptr;
		UMaterialInstanceConstant* Material = Cast<UMaterialInstanceConstant>(Path.TryLoad());
		return Material && UsedMaterials.Contains(Material) ? Material : nullptr;
	};
	if (bUseKeywordMaterials)
	{
		if (!bUpdateKeywordMaterialsOnReimport) return;
		for (const auto& Pair : KeywordFactoryRows)
		{
			if (UMaterialInstanceConstant* Material = FindMaterial(Pair.Key))
				ApplyKeywordMaterial(Container, Pair.Key, Material, true);
		}
		return;
	}
	if (bConfigureFaceMaterial && bUpdateFaceMaterialOnReimport)
	{
		for (const FString& Key : FaceFactoryKeys)
		{
			if (UMaterialInstanceConstant* Material = FindMaterial(Key)) ApplyFaceMaterial(Container, Key, Material, true);
		}
	}
	if (bConfigureBodyMaterial && bUpdateBodyMaterialOnReimport)
	{
		for (const auto& Pair : BodyFactoryRows)
		{
			if (UMaterialInstanceConstant* Material = FindMaterial(Pair.Key)) ApplyBodyMaterial(Container, Pair.Key, Material, true);
		}
	}
	if (bReapplyMaterialRulesOnReimport)
	{
		for (const auto& Pair : MaterialRuleIndices)
		{
			if (UMaterialInstanceConstant* Material = FindMaterial(Pair.Key)) ApplyLegacyMaterial(Container, Pair.Key, Material, Pair.Value);
		}
	}
}
