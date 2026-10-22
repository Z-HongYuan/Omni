// Copyright © 2026 张鸿源. All Rights Reserved.
#include "Import/OmniCharacterImportPipeline.h"
#include "Import/OmniCharacterImportProfile.h"
#include "Import/Materials/OmniImportToonProfile.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/ToonProfile.h"
#include "InterchangeMaterialFactoryNode.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Nodes/InterchangeBaseNodeContainer.h"

namespace
{
	void AddMeshMaterials(UObject* Asset, TArray<TObjectPtr<UMaterialInstanceConstant>>& Materials)
	{
		auto Add = [&Materials](UMaterialInterface* Material)
		{
			if (UMaterialInstanceConstant* Instance = Cast<UMaterialInstanceConstant>(Material)) Materials.AddUnique(Instance);
		};
		if (UMaterialInstanceConstant* Instance = Cast<UMaterialInstanceConstant>(Asset)) Add(Instance);
		else if (const USkeletalMesh* Mesh = Cast<USkeletalMesh>(Asset))
			for (const FSkeletalMaterial& Slot : Mesh->GetMaterials()) Add(Slot.MaterialInterface);
		else if (const UStaticMesh* StaticMesh = Cast<UStaticMesh>(Asset))
			for (const FStaticMaterial& Slot : StaticMesh->GetStaticMaterials()) Add(Slot.MaterialInterface);
	}
}

void UOmniCharacterImportPipeline::PrepareSharedToonProfile(const UInterchangeBaseNodeContainer* Container)
{
	SharedToonProfile = nullptr;
	SharedToonProfileTemplate = nullptr;
	bSharedToonProfileResolved = false;
	SharedProfileCandidates.Reset();
	SharedToonProfilePackage = CharacterRoot / ActiveProfile->MaterialFolder / (TEXT("TP_") + ResolvedName);
	// 重导入替换材质槽前记录旧材质；即使新 MI 先回调，也能找回旧 MI 持有的已重命名组 Profile。
	AddMeshMaterials(ToonProfileReimportAsset, SharedProfileCandidates);
	if (!BodyToonProfileTemplate.IsNull())
	{
		SharedToonProfileTemplate = BodyToonProfileTemplate.LoadSynchronous();
		if (!SharedToonProfileTemplate) Report(TEXT("The asset Toon Profile template could not be loaded."), true);
		return;
	}
	if (bUseKeywordMaterials)
	{
		TArray<FOmniKeywordMaterialBinding> Candidates = MaterialMappings;
		Candidates.Sort([](const FOmniKeywordMaterialBinding& A, const FOmniKeywordMaterialBinding& B)
		{
			const bool ABody = A.ResolvedRole == EOmniMaterialRole::Body;
			const bool BBody = B.ResolvedRole == EOmniMaterialRole::Body;
			return ABody != BBody ? ABody : A.SourceUid < B.SourceUid;
		});
		for (const FOmniKeywordMaterialBinding& Row : Candidates)
		{
			SharedToonProfileTemplate = Omni::CharacterImport::ResolveBodyToonProfileTemplate(Row.Parent.LoadSynchronous(), nullptr);
			if (SharedToonProfileTemplate) break;
		}
	}
	else
	{
		if (bConfigureBodyMaterial && !BodyFactoryRows.IsEmpty())
			SharedToonProfileTemplate = Omni::CharacterImport::ResolveBodyToonProfileTemplate(BodyParentMaterial.LoadSynchronous(), nullptr);
		if (!SharedToonProfileTemplate && bConfigureFaceMaterial && !FaceFactoryKeys.IsEmpty())
			SharedToonProfileTemplate = Omni::CharacterImport::ResolveBodyToonProfileTemplate(FaceParentMaterial.LoadSynchronous(), nullptr);
		if (!SharedToonProfileTemplate && ActiveProfile->bConfigureMaterials)
		{
			TArray<UInterchangeMaterialInstanceFactoryNode*> Nodes;
			Container->GetNodesOfType(Nodes);
			Nodes.Sort([](const UInterchangeMaterialInstanceFactoryNode& A, const UInterchangeMaterialInstanceFactoryNode& B)
			{
				return A.GetUniqueID() < B.GetUniqueID();
			});
			for (const UInterchangeMaterialInstanceFactoryNode* Node : Nodes)
			{
				if (FaceFactoryKeys.Contains(Node->GetUniqueID()) || BodyFactoryRows.Contains(Node->GetUniqueID())) continue;
				const FOmniImportMaterialRule* Rule = ActiveProfile->MaterialRules.FindByPredicate([Node](const FOmniImportMaterialRule& Item)
				{
					return !Item.MaterialNamePattern.IsEmpty() &&
						Node->GetDisplayLabel().MatchesWildcard(Item.MaterialNamePattern, ESearchCase::IgnoreCase);
				});
				if (!Rule) continue;
				SharedToonProfileTemplate = Omni::CharacterImport::ResolveBodyToonProfileTemplate(Rule->ParentMaterial.LoadSynchronous(), nullptr);
				if (SharedToonProfileTemplate) break;
			}
		}
	}
}

void UOmniCharacterImportPipeline::ApplyAssetToonProfile(const UInterchangeBaseNodeContainer* Container,
	UMaterialInstanceConstant* Material)
{
	if (!Container || !Material || (!BodyToonProfileTemplate.IsNull() && !SharedToonProfileTemplate)) return;
	if (!bSharedToonProfileResolved)
	{
		SharedProfileCandidates.AddUnique(Material);
		TArray<UInterchangeMaterialInstanceFactoryNode*> Nodes;
		Container->GetNodesOfType(Nodes);
		for (const UInterchangeMaterialInstanceFactoryNode* Node : Nodes)
		{
			FSoftObjectPath Path;
			if (Node->GetCustomReferenceObject(Path))
				if (UMaterialInstanceConstant* Existing = Cast<UMaterialInstanceConstant>(Path.TryLoad())) SharedProfileCandidates.AddUnique(Existing);
		}
		TArray<UMaterialInstanceConstant*> ExistingMaterials;
		for (UMaterialInstanceConstant* Existing : SharedProfileCandidates) ExistingMaterials.Add(Existing);
		SharedToonProfile = Omni::CharacterImport::ResolveOrCreateSharedToonProfile(SharedToonProfilePackage,
			ExistingMaterials, SharedToonProfileTemplate, {[this](const FString& Message, bool bError) { Report(Message, bError); }});
		bSharedToonProfileResolved = true;
	}
	// 未指定模板时清理导入器拥有的覆盖；模板解析失败时保留现有绑定。
	if (SharedToonProfileTemplate && !SharedToonProfile) return;
	Omni::CharacterImport::ApplySharedToonProfile(Material, SharedToonProfile, SharedToonProfilePackage,
		{[this](const FString& Message, bool bError) { Report(Message, bError); }});
}

void UOmniCharacterImportPipeline::ApplyAssetToonProfiles(const UInterchangeBaseNodeContainer* Container,
	UObject* Mesh, const bool bReimport)
{
	if (!Container) return;
	const bool Reimport = bReimport || bIsReimportContext;
	TArray<FString> Keys;
	if (bUseKeywordMaterials)
	{
		if (Reimport && !bUpdateKeywordMaterialsOnReimport) return;
		KeywordFactoryRows.GetKeys(Keys);
	}
	else
	{
		if (bConfigureBodyMaterial && (!Reimport || bUpdateBodyMaterialOnReimport)) BodyFactoryRows.GetKeys(Keys);
		if (bConfigureFaceMaterial && (!Reimport || bUpdateFaceMaterialOnReimport)) Keys.Append(FaceFactoryKeys.Array());
		if (!Reimport || bReapplyMaterialRulesOnReimport)
		{
			TArray<FString> RuleKeys;
			MaterialRuleIndices.GetKeys(RuleKeys);
			Keys.Append(RuleKeys);
		}
	}

	TArray<TObjectPtr<UMaterialInstanceConstant>> UsedMaterials;
	AddMeshMaterials(Mesh, UsedMaterials);
	for (const FString& Key : Keys)
	{
		const UInterchangeFactoryBaseNode* Node = Container->GetFactoryNode(Key);
		FSoftObjectPath Path;
		if (!Node || !Node->GetCustomReferenceObject(Path)) continue;
		UMaterialInstanceConstant* Material = Cast<UMaterialInstanceConstant>(Path.TryLoad());
		if (Material && UsedMaterials.Contains(Material)) ApplyAssetToonProfile(Container, Material);
	}
}
