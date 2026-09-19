// Copyright © 2026 张鸿源. All Rights Reserved.
#include "Import/OmniCharacterImportPipeline.h"

#include "Import/OmniCharacterImportProfile.h"
#include "Import/Materials/OmniImportMaterialResolver.h"
#include "InterchangeMaterialInstanceNode.h"
#include "InterchangeShaderGraphNode.h"
#include "InterchangeSkeletalMeshFactoryNode.h"
#include "InterchangeTextureFactoryNode.h"
#include "InterchangeTextureNode.h"
#include "Misc/PackageName.h"
#include "Nodes/InterchangeBaseNodeContainer.h"
#include "ObjectTools.h"

namespace
{
	FString TexturePrefix(const FString& Name)
	{
		return Name.StartsWith(TEXT("T_"), ESearchCase::IgnoreCase) ? TEXT("T_") + Name.Mid(2) : TEXT("T_") + Name;
	}

	FString MaterialSection(FString Label, const FString& Character, const FString& Fallback = FString())
	{
		Label = ObjectTools::SanitizeObjectName(Label);
		Label.ReplaceInline(TEXT("-"), TEXT("_"));
		if (Label.Contains(TEXT("Outline"))) return FString();
		Label.RemoveFromStart(TEXT("MI_"));
		Label.RemoveFromStart(TEXT("M_"));
		Label.RemoveFromStart(Character + TEXT("_"));
		TArray<FString> Tokens;
		Label.ParseIntoArray(Tokens, TEXT("_"), true);
		TArray<FString> CharacterTokens;
		Character.ParseIntoArray(CharacterTokens, TEXT("_"), true);
		// Roles come from material names/bindings, never from texture file names.
		for (int32 Index = 0; Index < Tokens.Num(); ++Index)
		{
			for (const FString Role : {TEXT("EyeStar"), TEXT("Body"), TEXT("Face"), TEXT("Hair"), TEXT("Eye"), TEXT("Brow"), TEXT("Mouth"), TEXT("Glasses"), TEXT("Accessory")})
			{
				if (!Tokens[Index].Equals(Role, ESearchCase::IgnoreCase)) continue;
				FString Result = Role;
				for (int32 Detail = Index + 1; Detail < Tokens.Num(); ++Detail) Result += TEXT("_") + Tokens[Detail];
				// Preserve variant labels in legacy names such as Original_Lumine_Body.
				for (int32 Detail = 0; Detail < Index; ++Detail)
				{
					if (!CharacterTokens.ContainsByPredicate([&](const FString& Word) { return Word.Equals(Tokens[Detail], ESearchCase::IgnoreCase); }))
						Result += TEXT("_") + Tokens[Detail];
				}
				return Result;
			}
		}
		return Fallback;
	}

	struct FTextureUses
	{
		TSet<FString> Sections;
		TSet<FString> Channels;
		TSet<FString> Labels;
	};
}

void UOmniCharacterImportPipeline::RefreshTextureNames(UInterchangeBaseNodeContainer* Container, const FString& Character)
{
	TextureNamingError.Reset();
	if (!bNormalizeTextureNames)
	{
		TextureNamingStatus = TEXT("沿用源纹理名称和 T_ 前缀。");
		return;
	}
	if (bIsReimportContext)
	{
		TextureNamingStatus = TEXT("重导入保留已有纹理名称、路径和引用；本次不重新命名。");
		return;
	}
	if (!Container)
	{
		TextureNamingStatus = TEXT("选择 FBX 后显示纹理名称预览；不会重命名项目已有纹理。");
		return;
	}
	FString CharacterLabel = Character.IsEmpty() ? CharacterName : Character;
	if (CharacterLabel.IsEmpty())
	{
		TArray<UInterchangeSkeletalMeshFactoryNode*> Meshes;
		Container->GetNodesOfType(Meshes);
		if (Meshes.Num() == 1)
		{
			CharacterLabel = Meshes[0]->GetAssetName();
			CharacterLabel.RemoveFromStart(TEXT("SKM_"));
		}
	}
	// Dialog translation can precede the base pipeline's asset-name resolution.
	if (CharacterLabel.IsEmpty()) CharacterLabel = TEXT("{Character}");
	TMap<FString, FTextureUses> Uses;
	auto AddUse = [&Uses](const FString& Uid, const FString& Section, FString Channel)
	{
		if (Uid.IsEmpty() || Section.IsEmpty()) return;
		Channel.RemoveFromStart(TEXT("Tex_"));
		if (Channel.IsEmpty()) return;
		FTextureUses& Use = Uses.FindOrAdd(Uid);
		Use.Sections.Add(Section);
		Use.Channels.Add(Channel);
		Use.Labels.Add(Section + TEXT(" → ") + Channel);
	};
	if (bUseKeywordMaterials)
	{
		for (const FOmniKeywordMaterialBinding& Section : MaterialMappings)
		{
			for (const FOmniFaceTextureBinding& Binding : Section.Textures)
			{
				const FString Group = MaterialTextureGroup(Section, Binding.Parameter);
				AddUse(Binding.ResolvedTextureUid, Group.IsEmpty() ? Section.AssetSuffix : Group, Binding.Parameter.ToString());
			}
		}
	}
	else
	{
	TArray<UInterchangeBaseNode*> Nodes;
	Container->GetNodesOfType(Nodes);
	for (const UInterchangeBaseNode* Node : Nodes)
	{
		if (!Node->IsA<UInterchangeShaderGraphNode>() && !Node->IsA<UInterchangeMaterialInstanceNode>()) continue;
		const FString Section = (bConfigureFaceMaterial && Node->GetUniqueID() == FaceSourceUid)
			? TEXT("Face") : MaterialSection(Node->GetDisplayLabel(), CharacterLabel);
		if (Section.IsEmpty()) continue;
		for (const FName Parameter : {FName(TEXT("Tex_BaseColor")), FName(TEXT("Tex_Normal"))})
		{
			FOmniFaceTextureBinding Binding;
			Binding.Parameter = Parameter;
			Binding.Mode = EOmniFaceTextureMode::AutoMatch;
			TArray<FOmniFaceTextureBinding> Bindings{Binding};
			FString Error;
			// This role disables Face's optional filename fallback: only source links count here.
			Omni::CharacterImport::ResolveMaterialTextures(Container, Node, TEXT("TextureNaming"), Bindings, Error);
			if (Error.IsEmpty()) AddUse(Bindings[0].ResolvedTextureUid, Section, Parameter.ToString());
		}
	}
	if (bConfigureFaceMaterial && !FaceSourceUid.IsEmpty())
	{
		for (const FOmniFaceTextureBinding& Binding : FaceTextures)
			AddUse(Binding.ResolvedTextureUid, TEXT("Face"), Binding.Parameter.ToString());
	}
	if (bConfigureBodyMaterial)
	{
		for (const FOmniBodyMaterialBinding& Section : BodyMaterials)
		{
			if (!Section.bEnabled || Section.ResolvedMaterialUid.IsEmpty()) continue;
			const FString Label = MaterialSection(Section.SourceMaterial, CharacterLabel, TEXT("Body"));
			for (const FOmniFaceTextureBinding& Binding : Section.Textures)
				AddUse(Binding.ResolvedTextureUid, Label, Binding.Parameter.ToString());
		}
	}
	}
	TArray<UInterchangeTextureNode*> Textures;
	Container->GetNodesOfType(Textures);
	// Context setup can run before translation; keep explicit overrides until source nodes exist.
	if (Textures.IsEmpty()) return;
	Textures.Sort([](const UInterchangeTextureNode& A, const UInterchangeTextureNode& B)
	{
		const int32 Order = A.GetDisplayLabel().Compare(B.GetDisplayLabel(), ESearchCase::IgnoreCase);
		return Order == 0 ? A.GetUniqueID() < B.GetUniqueID() : Order < 0;
	});
	TArray<FOmniTextureNameBinding> Rows;
	for (const UInterchangeTextureNode* Texture : Textures)
	{
		FOmniTextureNameBinding Row;
		const FOmniTextureNameBinding* Previous = TextureNames.FindByPredicate([Texture](const FOmniTextureNameBinding& Existing)
		{
			return Existing.SourceUid == Texture->GetUniqueID() ||
				(Existing.SourceUid.IsEmpty() && Existing.SourceTexture == Texture->GetDisplayLabel());
		});
		if (Previous) Row = *Previous;
		Row.SourceUid = Texture->GetUniqueID();
		Row.SourceTexture = Texture->GetDisplayLabel();
		Row.Usage.Reset();
		Row.SuggestedName = TexturePrefix(ObjectTools::SanitizeObjectName(Row.SourceTexture));
		Row.Status = TEXT("用途不明确，保留源名称；可填写自定义名称。");
		if (bUseKeywordMaterials && !Uses.Contains(Row.SourceUid))
		{
			const TSet<FName> Parameters = TextureParametersFromKeywords(Row.SourceTexture);
			if (Parameters.Num() == 1)
			{
				const FString Group = SourceTextureGroup(Row.SourceTexture);
				AddUse(Row.SourceUid, Group.IsEmpty() ? TEXT("Shared") : Group, Parameters.CreateConstIterator()->ToString());
			}
		}
		if (const FTextureUses* Use = Uses.Find(Row.SourceUid))
		{
			TArray<FString> Labels = Use->Labels.Array();
			Labels.Sort();
			Row.Usage = FString::Join(Labels, TEXT("；"));
			if (Use->Channels.Num() == 1)
			{
				const FString Section = Use->Sections.Num() == 1 ? *Use->Sections.CreateConstIterator() : TEXT("Shared");
				Row.SuggestedName = TEXT("T_") + CharacterLabel + TEXT("_") + Section + TEXT("_") + *Use->Channels.CreateConstIterator();
				Row.Status = Use->Sections.Num() == 1 ? TEXT("按材质用途命名。") : TEXT("多个材质共用，只创建一个纹理资产。");
			}
			else Row.Status = TEXT("同一纹理用于多个参数，保留源名称；可填写自定义名称。");
		}
		Row.FinalName = Row.CustomName.IsEmpty() ? Row.SuggestedName : TexturePrefix(Row.CustomName);
		if (!Row.CustomName.IsEmpty() && (Row.CustomName.TrimStartAndEnd() != Row.CustomName ||
			ObjectTools::SanitizeObjectName(Row.CustomName) != Row.CustomName || Row.CustomName.Contains(TEXT("/"))))
		{
			Row.Status = TEXT("自定义名称不能包含路径、空格或非法字符。");
			TextureNamingError = Row.SourceTexture + TEXT("：") + Row.Status;
		}
		Rows.Add(MoveTemp(Row));
	}
	// Reserve explicit names and distinct suggestions before assigning deterministic suffixes.
	TSet<FString> ReservedNames;
	TSet<FString> ManualNames;
	for (const auto& Row : Rows)
	{
		ReservedNames.Add(Row.FinalName.ToLower());
		if (!Row.CustomName.IsEmpty()) ManualNames.Add(Row.FinalName.ToLower());
	}
	TMap<FString, int32> Seen;
	for (int32 Index = 0; Index < Rows.Num(); ++Index)
	{
		FOmniTextureNameBinding& Row = Rows[Index];
		if (Row.CustomName.IsEmpty() && (Seen.Contains(Row.FinalName.ToLower()) || ManualNames.Contains(Row.FinalName.ToLower())))
		{
			const FString Base = Row.FinalName;
			int32 Suffix = 2;
			do { Row.FinalName = Base + FString::Printf(TEXT("_%02d"), Suffix++); }
			while (Seen.Contains(Row.FinalName.ToLower()) || ReservedNames.Contains(Row.FinalName.ToLower()));
			Row.Status = TEXT("自动名称重复，已追加稳定序号：") + Row.FinalName;
		}
		const FString Key = Row.FinalName.ToLower();
		if (const int32* Other = Seen.Find(Key))
		{
			Row.Status = Rows[*Other].Status = TEXT("不同源纹理的目标名称冲突，请填写不同的自定义名称。");
			TextureNamingError = Row.FinalName + TEXT("：") + Row.Status;
		}
		else Seen.Add(Key, Index);
	}
	TextureNames = MoveTemp(Rows);
	TextureNamingStatus = TextureNamingError.IsEmpty()
		? TEXT("按材质连接、关键词及 Tex_ 参数命名；{Character} 在导入时替换为基础 FBX 管线的角色名。未识别的纹理保留源名称。")
		: TextureNamingError;
}

bool UOmniCharacterImportPipeline::ValidateTextureNames(TOptional<FText>& OutInvalidReason) const
{
	if (!bNormalizeTextureNames || bIsReimportContext || TextureNamingError.IsEmpty()) return true;
	OutInvalidReason = FText::FromString(TextureNamingError);
	return false;
}

bool UOmniCharacterImportPipeline::PrepareTextureNames(UInterchangeBaseNodeContainer* Container)
{
	if (!bNormalizeTextureNames || bIsReimportContext) return true;
	if (!TextureNamingError.IsEmpty())
	{
		Report(TextureNamingError, true);
		return false;
	}
	// Validate every destination before any factory can create assets.
	for (FOmniTextureNameBinding& Row : TextureNames)
	{
		const FString Uid = UInterchangeFactoryBaseNode::BuildFactoryNodeUid(Row.SourceUid);
		if (!Cast<UInterchangeTextureFactoryNode>(Container->GetFactoryNode(Uid))) continue;
		const FString Package = CharacterRoot / ActiveProfile->TextureFolder / Row.FinalName;
		if (!FPackageName::IsValidLongPackageName(Package) || FPackageName::DoesPackageExist(Package) ||
			StaticFindObject(UObject::StaticClass(), nullptr, *(Package + TEXT(".") + Row.FinalName)))
		{
			Row.Status = TEXT("目标路径无效或已有资产占用；请改名，或对已有资产执行重导入。");
			Report(Package + TEXT("：") + Row.Status, true);
			return false;
		}
	}
	for (const FOmniTextureNameBinding& Row : TextureNames)
	{
		const FString Uid = UInterchangeFactoryBaseNode::BuildFactoryNodeUid(Row.SourceUid);
		if (UInterchangeTextureFactoryNode* Factory = Cast<UInterchangeTextureFactoryNode>(Container->GetFactoryNode(Uid)))
		{
			Factory->SetAssetName(Row.FinalName);
			Report(TEXT("Texture name: ") + Row.SourceTexture + TEXT(" -> ") + Row.FinalName);
		}
	}
	return true;
}
