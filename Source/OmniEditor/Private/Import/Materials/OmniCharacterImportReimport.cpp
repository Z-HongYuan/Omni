// Copyright © 2026 张鸿源. All Rights Reserved.
#include "Import/OmniCharacterImportPipeline.h"

#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Nodes/InterchangeBaseNodeContainer.h"
#include "Nodes/InterchangeFactoryBaseNode.h"

void UOmniCharacterImportPipeline::ApplyRetainedMaterials(const UInterchangeBaseNodeContainer* Container, USkeletalMesh* Mesh)
{
	if (!Container || !Mesh) return;
	// Mesh-only reimport can retain a material without dispatching its post-import callback.
	// Restrict updates to factory references actually used by this mesh, never name-based searches.
	auto FindMaterial = [Container, Mesh](const FString& Key) -> UMaterialInstanceConstant*
	{
		const UInterchangeFactoryBaseNode* Node = Container->GetFactoryNode(Key);
		FSoftObjectPath Path;
		if (!Node || !Node->GetCustomReferenceObject(Path)) return nullptr;
		UMaterialInstanceConstant* Material = Cast<UMaterialInstanceConstant>(Path.TryLoad());
		return Material && Mesh->GetMaterials().ContainsByPredicate([Material](const FSkeletalMaterial& Slot)
		{
			return Slot.MaterialInterface == Material;
		}) ? Material : nullptr;
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
