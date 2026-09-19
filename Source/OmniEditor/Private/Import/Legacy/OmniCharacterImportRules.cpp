// Copyright © 2026 张鸿源. All Rights Reserved.
#include "Import/OmniCharacterImportPipeline.h"
#include "Import/OmniCharacterImportProfile.h"
#include "Import/Materials/OmniImportMaterialApplier.h"
#include "Engine/Texture.h"
#include "InterchangeMaterialFactoryNode.h"
#include "InterchangeTextureFactoryNode.h"
#include "InterchangeShaderGraphNode.h"
#include "MaterialEditingLibrary.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Nodes/InterchangeBaseNodeContainer.h"

namespace
{
bool MatchesPattern(const FString& Name, const FString& Pattern)
{
	return !Pattern.IsEmpty() && Name.MatchesWildcard(Pattern, ESearchCase::IgnoreCase);
}
}

bool UOmniCharacterImportPipeline::ValidateLegacyMaterialRules(const UOmniCharacterImportProfile* Settings, TOptional<FText>& OutInvalidReason) const
{
	if (Settings->bConfigureMaterials)
	{
		for (const FOmniImportMaterialRule& Rule : Settings->MaterialRules)
		{
			if (Rule.MaterialNamePattern.IsEmpty() || Rule.ParentMaterial.IsNull())
			{
				OutInvalidReason = FText::FromString(TEXT("Each material rule needs a name pattern and a parent material."));
				return false;
			}
			TSet<FName> Parameters;
			for (const FOmniImportTextureBinding& Binding : Rule.Textures)
			{
				if (Binding.Parameter.IsNone() || Parameters.Contains(Binding.Parameter) ||
					(Binding.TextureNamePattern.IsEmpty() && Binding.ExistingTexture.IsNull()))
				{
					OutInvalidReason = FText::FromString(TEXT("Texture bindings need unique parameter names and either an existing texture or a source texture pattern."));
					return false;
				}
				Parameters.Add(Binding.Parameter);
			}
		}
	}
	return true;
}

void UOmniCharacterImportPipeline::PrepareLegacyMaterials(UInterchangeBaseNodeContainer* Container)
{
	TArray<UInterchangeTextureFactoryNode*> Textures;
	Container->GetNodesOfType(Textures);
	TMap<FString, EOmniImportTextureUsage> TextureUsages;
	Container->IterateNodesOfType<UInterchangeMaterialInstanceFactoryNode>([&](const FString& Key, UInterchangeMaterialInstanceFactoryNode* Node)
	{
		if (FaceFactoryKeys.Contains(Key) || BodyFactoryRows.Contains(Key)) return; // Dialog settings take precedence.
		const int32 RuleIndex = ActiveProfile->MaterialRules.IndexOfByPredicate([Node](const FOmniImportMaterialRule& Rule)
		{
			return MatchesPattern(Node->GetDisplayLabel(), Rule.MaterialNamePattern);
		});
		if (RuleIndex == INDEX_NONE) return;
		const FOmniImportMaterialRule& Rule = ActiveProfile->MaterialRules[RuleIndex];
		UMaterialInterface* Parent = Rule.ParentMaterial.LoadSynchronous();
		if (!Parent)
		{
			Report(TEXT("Missing parent material for ") + Node->GetDisplayLabel(), true);
			return;
		}
		MaterialRuleIndices.Add(Key, RuleIndex);
		Node->SetCustomParent(Parent->GetPathName());
		TArray<FMaterialParameterInfo> ParameterInfos;
		TArray<FGuid> ParameterIds;
		Parent->GetAllTextureParameterInfo(ParameterInfos, ParameterIds);
		for (const FOmniImportTextureBinding& Binding : Rule.Textures)
		{
			if (!ParameterInfos.ContainsByPredicate([&](const FMaterialParameterInfo& Info) { return Info.Name == Binding.Parameter; }))
			{
				Report(FString::Printf(TEXT("Missing texture parameter %s in %s"), *Binding.Parameter.ToString(), *Parent->GetName()), true);
				continue;
			}
			if (!Binding.ExistingTexture.IsNull()) continue; // Applied after the instance exists.
			TArray<UInterchangeTextureFactoryNode*> Matches = Textures.FilterByPredicate([&](const UInterchangeTextureFactoryNode* Texture)
			{
				return MatchesPattern(Texture->GetDisplayLabel(), Binding.TextureNamePattern);
			});
			if (Matches.Num() != 1)
			{
				Report(FString::Printf(TEXT("Texture pattern '%s' matched %d sources for %s; binding left unchanged."), *Binding.TextureNamePattern, Matches.Num(), *Node->GetDisplayLabel()), true);
				continue;
			}
			UInterchangeTextureFactoryNode* Texture = Matches[0];
			const FString TextureKey = Texture->GetUniqueID();
			Node->AddStringAttribute(UInterchangeShaderPortsAPI::MakeInputParameterKey(Binding.Parameter.ToString()), TextureKey);
			Node->AddFactoryDependencyUid(TextureKey);
			if (Binding.Usage != EOmniImportTextureUsage::Preserve)
			{
				if (const EOmniImportTextureUsage* Existing = TextureUsages.Find(TextureKey); Existing && *Existing != Binding.Usage)
				{
					Report(TEXT("Conflicting texture usages: ") + Texture->GetDisplayLabel(), true);
					continue;
				}
				TextureUsages.Add(TextureKey, Binding.Usage);
				Texture->SetCustomSRGB(Binding.Usage == EOmniImportTextureUsage::Color);
				const uint8 Compression = Binding.Usage == EOmniImportTextureUsage::Normal ? TC_Normalmap : TC_Default;
				Texture->SetCustomCompressionSettings(Compression);
			}
		}
	});
}

void UOmniCharacterImportPipeline::ApplyLegacyMaterial(const UInterchangeBaseNodeContainer* Container, const FString& NodeKey,
	UMaterialInstanceConstant* Material, int32 RuleIndex)
{
	const FOmniImportMaterialRule& Rule = ActiveProfile->MaterialRules[RuleIndex];
	if (UMaterialInterface* Parent = Rule.ParentMaterial.LoadSynchronous()) Material->SetParentEditorOnly(Parent);
	for (const FOmniImportTextureBinding& Binding : Rule.Textures)
	{
		UTexture* Texture = Binding.ExistingTexture.LoadSynchronous();
		if (Binding.ExistingTexture.IsNull() && Container)
		{
			const UInterchangeFactoryBaseNode* MaterialNode = Container->GetFactoryNode(NodeKey);
			FString TextureNodeKey;
			if (MaterialNode && MaterialNode->GetStringAttribute(UInterchangeShaderPortsAPI::MakeInputParameterKey(Binding.Parameter.ToString()), TextureNodeKey))
			{
				if (const UInterchangeFactoryBaseNode* TextureNode = Container->GetFactoryNode(TextureNodeKey))
				{
					FSoftObjectPath TexturePath;
					TextureNode->GetCustomReferenceObject(TexturePath);
					Texture = Cast<UTexture>(TexturePath.TryLoad());
				}
			}
			if (!Texture) continue; // Missing/ambiguous source was already reported during planning.
		}
		if (!Omni::CharacterImport::SetTextureParameterVerified(Material, Binding.Parameter, Texture))
			Report(TEXT("Could not bind texture parameter: ") + Binding.Parameter.ToString(), true);
	}
	for (const TPair<FName, float>& Parameter : Rule.ScalarParameters)
	{
		if (!UMaterialEditingLibrary::SetMaterialInstanceScalarParameterValue(Material, Parameter.Key, Parameter.Value))
			Report(TEXT("Unknown scalar parameter: ") + Parameter.Key.ToString(), true);
	}
	for (const TPair<FName, bool>& Parameter : Rule.StaticSwitchParameters)
	{
		if (!UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(Material, Parameter.Key, Parameter.Value,
		                                                                            EMaterialParameterAssociation::GlobalParameter, false))
			Report(TEXT("Unknown static switch: ") + Parameter.Key.ToString(), true);
	}
	UMaterialEditingLibrary::UpdateMaterialInstance(Material);
	(void)Material->MarkPackageDirty(); // Asset saving is controlled by the editor/user.
}
