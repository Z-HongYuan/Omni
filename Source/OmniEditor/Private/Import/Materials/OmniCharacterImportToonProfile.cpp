// Copyright © 2026 张鸿源. All Rights Reserved.
#include "Import/OmniCharacterImportPipeline.h"
#include "Import/Materials/OmniImportToonProfile.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/ToonProfile.h"
#include "InterchangeMaterialFactoryNode.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Nodes/InterchangeBaseNodeContainer.h"

UToonProfile* UOmniCharacterImportPipeline::ResolveBodyToonProfileTemplate() const
{
	if (!BodyToonProfileTemplate.IsNull()) return BodyToonProfileTemplate.LoadSynchronous();
	return Omni::CharacterImport::ResolveBodyToonProfileTemplate(BodyParentMaterial.LoadSynchronous(), nullptr);
}

void UOmniCharacterImportPipeline::ApplyBodyToonProfile(const UInterchangeBaseNodeContainer* Container,
                                                        USkeletalMesh* Mesh, const bool bReimport)
{
	if (!Container || (!bUseKeywordMaterials && !bConfigureBodyMaterial) ||
		(bReimport && !(bUseKeywordMaterials ? bUpdateKeywordMaterialsOnReimport : bUpdateBodyMaterialOnReimport))) return;
	TSet<UMaterialInstanceConstant*> Targets;
	const TMap<FString, int32>& Rows = bUseKeywordMaterials ? KeywordFactoryRows : BodyFactoryRows;
	for (const TPair<FString, int32>& Pair : Rows)
	{
		if (bUseKeywordMaterials && (!MaterialMappings.IsValidIndex(Pair.Value) || MaterialMappings[Pair.Value].ResolvedRole != EOmniMaterialRole::Body)) continue;
		const UInterchangeFactoryBaseNode* Node = Container->GetFactoryNode(Pair.Key);
		FSoftObjectPath Path;
		if (!Node || !Node->GetCustomReferenceObject(Path)) continue;
		UMaterialInstanceConstant* Material = Cast<UMaterialInstanceConstant>(Path.TryLoad());
		if (Material && Mesh->GetMaterials().ContainsByPredicate([Material](const FSkeletalMaterial& Slot) { return Slot.MaterialInterface == Material; }))
		{
			Targets.Add(Material);
		}
	}
	if (Targets.IsEmpty()) return;
	Omni::CharacterImport::ApplyBodyToonProfile(Targets, Mesh, ResolveBodyToonProfileTemplate(),
		{[this](const FString& Message, bool bError) { Report(Message, bError); }});
}
