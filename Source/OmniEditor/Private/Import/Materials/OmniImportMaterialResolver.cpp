// Copyright © 2026 张鸿源. All Rights Reserved.
#include "Import/Materials/OmniImportMaterialResolver.h"
#include "Engine/Texture.h"
#include "InterchangeMaterialInstanceNode.h"
#include "InterchangeShaderGraphNode.h"
#include "InterchangeTextureNode.h"
#include "Materials/MaterialInterface.h"
#include "Nodes/InterchangeBaseNodeContainer.h"

namespace Omni::CharacterImport
{
	TArray<UInterchangeBaseNode*> GetSourceMaterials(const UInterchangeBaseNodeContainer* Container)
	{
		TArray<UInterchangeBaseNode*> Nodes;
		Container->GetNodesOfType(Nodes);
		return Nodes.FilterByPredicate([](const UInterchangeBaseNode* Node)
		{
			return Node->IsA<UInterchangeShaderGraphNode>() || Node->IsA<UInterchangeMaterialInstanceNode>();
		});
	}


	// Follow only the source material's color branch, never search another character's assets.
	void GatherTextureInputs(const UInterchangeBaseNodeContainer* Container, const FString& Uid, TSet<FString>& Visited, TSet<FString>& Textures)
	{
		if (Visited.Contains(Uid) || Visited.Num() >= 128) return;
		Visited.Add(Uid);
		const UInterchangeBaseNode* Node = Container->GetNode(Uid);
		if (!Node) return;
		if (Node->IsA<UInterchangeTextureNode>())
		{
			Textures.Add(Uid);
			return;
		}
		for (const FString& Key : {
			     UInterchangeShaderPortsAPI::MakeInputValueKey(TEXT("Texture")),
			     UInterchangeShaderPortsAPI::MakeInputParameterKey(TEXT("Texture"))
		     })
		{
			FString TextureUid;
			if (Node->GetStringAttribute(Key, TextureUid) && Cast<UInterchangeTextureNode>(Container->GetNode(TextureUid))) Textures.Add(TextureUid);
		}
		TArray<FString> Inputs;
		UInterchangeShaderPortsAPI::GatherInputs(Node, Inputs);
		for (const FString& Input : Inputs)
		{
			FString Connected;
			FString Output;
			if (UInterchangeShaderPortsAPI::GetInputConnection(Node, Input, Connected, Output)) GatherTextureInputs(Container, Connected, Visited, Textures);
		}
	}
}


void Omni::CharacterImport::RefreshMaterialTextureParameters(UMaterialInterface* Parent, TArray<FOmniFaceTextureBinding>& Bindings)
{
	if (!Parent) return;
	TArray<FMaterialParameterInfo> Infos;
	TArray<FGuid> Ids;
	Parent->GetAllTextureParameterInfo(Infos, Ids);
	TArray<FOmniFaceTextureBinding> Rows;
	for (const FMaterialParameterInfo& Info : Infos)
	{
		if (Info.Association != EMaterialParameterAssociation::GlobalParameter || !Info.Name.ToString().StartsWith(TEXT("Tex_"), ESearchCase::CaseSensitive)) continue;
		const FOmniFaceTextureBinding* Existing = Bindings.FindByPredicate([&](const FOmniFaceTextureBinding& Row) { return Row.Parameter == Info.Name; });
		FOmniFaceTextureBinding Row;
		if (Existing) Row = *Existing;
		else
		{
			Row.Parameter = Info.Name;
			if (Info.Name == TEXT("Tex_BaseColor")) Row.Mode = EOmniFaceTextureMode::AutoMatch;
		}
		UTexture* Default = nullptr;
		Parent->GetTextureParameterValue(Info, Default);
		Row.DefaultTexture = Default;
		Rows.Add(MoveTemp(Row));
	}
	Rows.Sort([](const FOmniFaceTextureBinding& A, const FOmniFaceTextureBinding& B) { return A.Parameter.LexicalLess(B.Parameter); });
	Bindings = MoveTemp(Rows);
}

void Omni::CharacterImport::ResolveMaterialTextures(const UInterchangeBaseNodeContainer* Container,
                                                           const UInterchangeBaseNode* SourceMaterial, const FString& RoleName,
                                                           TArray<FOmniFaceTextureBinding>& Bindings, FString& OutError)
{
	TArray<UInterchangeTextureNode*> Textures;
	Container->GetNodesOfType(Textures);
	for (FOmniFaceTextureBinding& Row : Bindings)
	{
		Row.ResolvedTextureUid.Reset();
		Row.bUseParentDefault = false;
		Row.Status.Reset();
		if (Row.Mode == EOmniFaceTextureMode::ParentDefault)
		{
			Row.Status = TEXT("沿用默认：") + (Row.DefaultTexture.IsNull() ? TEXT("未设置") : Row.DefaultTexture.GetAssetName());
			continue;
		}
		if (Row.Mode == EOmniFaceTextureMode::ExistingTexture)
		{
			if (Row.Texture.LoadSynchronous()) Row.Status = TEXT("手动指定：") + Row.Texture.GetAssetName();
			else
			{
				Row.Status = TEXT("请选择有效的项目纹理。");
				OutError = Row.Parameter.ToString() + TEXT("：") + Row.Status;
			}
			continue;
		}
		TSet<FString> Candidates;
		if (Row.Mode == EOmniFaceTextureMode::SourceTexture)
		{
			for (const UInterchangeTextureNode* Texture : Textures)
			{
				if (!Row.SourceTexture.IsEmpty() && Texture->GetDisplayLabel() == Row.SourceTexture) Candidates.Add(Texture->GetUniqueID());
			}
		}
		else
		{
			if (Row.MatchPattern.IsEmpty() && (Row.Parameter == TEXT("Tex_BaseColor") || Row.Parameter == TEXT("Tex_Normal")))
			{
				const TArray<FString> Inputs = Row.Parameter == TEXT("Tex_Normal")
					                               ? TArray<FString>{TEXT("Normal")}
					                               : TArray<FString>{TEXT("BaseColor"), TEXT("DiffuseColor")};
				for (const FString& Input : Inputs)
				{
					// UE 5.8 FBX stores texture references directly as *Map parameters.
					FString TextureUid;
					if (SourceMaterial->GetStringAttribute(UInterchangeShaderPortsAPI::MakeInputValueKey(Input + TEXT("Map")), TextureUid) &&
						Cast<UInterchangeTextureNode>(Container->GetNode(TextureUid)))
						Candidates.Add(TextureUid);
					FString Connected;
					FString Output;
					if (UInterchangeShaderPortsAPI::GetInputConnection(SourceMaterial, Input, Connected, Output))
					{
						TSet<FString> Visited;
						GatherTextureInputs(Container, Connected, Visited, Candidates);
					}
				}
			}
			if (Candidates.IsEmpty())
			{
				const FString Suffix = Row.Parameter.ToString().RightChop(4);
				for (const UInterchangeTextureNode* Texture : Textures)
				{
					const FString Label = Texture->GetDisplayLabel();
					const bool bMatch = !Row.MatchPattern.IsEmpty()
						                    ? Label.MatchesWildcard(Row.MatchPattern)
						                    : (Row.Parameter == TEXT("Tex_BaseColor")
							                       ? RoleName == TEXT("Face") && Label.Contains(TEXT("Face")) && (Label.Contains(TEXT("Diffuse")) || Label.Contains(TEXT("BaseColor")) || Label.Contains(TEXT("Albedo")))
							                       : RoleName == TEXT("Face") && Label.Contains(Suffix));
					if (bMatch) Candidates.Add(Texture->GetUniqueID());
				}
			}
		}
		if (Candidates.IsEmpty() && Row.Mode == EOmniFaceTextureMode::AutoMatch)
		{
			Row.bUseParentDefault = true;
			Row.Status = TEXT("未找到纹理，沿用母材质默认值。");
		}
		else if (Candidates.Num() == 1 || (Candidates.Num() > 1 && Row.Mode == EOmniFaceTextureMode::AutoMatch))
		{
			TArray<FString> Ordered = Candidates.Array();
			Ordered.Sort([Container](const FString& A, const FString& B)
			{
				const int32 Order = Container->GetNode(A)->GetDisplayLabel().Compare(Container->GetNode(B)->GetDisplayLabel(), ESearchCase::IgnoreCase);
				return Order == 0 ? A < B : Order < 0;
			});
			Row.ResolvedTextureUid = Ordered[0];
			Row.Status = Candidates.Num() == 1 ? TEXT("已匹配：") : FString::Printf(TEXT("%d 张候选，按源名称排序自动选择第一张："), Candidates.Num());
			Row.Status += Container->GetNode(Row.ResolvedTextureUid)->GetDisplayLabel();
		}
		else
		{
			Row.Status = FString::Printf(TEXT("匹配到 %d 张，请选择纹理或沿用母材质默认值。"), Candidates.Num());
			OutError = Row.Parameter.ToString() + TEXT("：") + Row.Status;
		}
	}
}
