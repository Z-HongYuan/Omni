// Copyright © 2026 张鸿源. All Rights Reserved.
#include "Import/OmniCharacterImportPipeline.h"

#include "Import/OmniCharacterImportProfile.h"
#include "Import/Materials/OmniImportMaterialResolver.h"
#include "Import/Materials/OmniImportMaterialApplier.h"
#include "Import/Materials/OmniImportToonProfile.h"
#include "Import/Materials/OmniImportTextureSettings.h"
#include "InterchangeMaterialFactoryNode.h"
#include "InterchangeMaterialInstanceNode.h"
#include "InterchangeShaderGraphNode.h"
#include "InterchangeTextureNode.h"
#include "Materials/MaterialInterface.h"
#include "Misc/PackageName.h"
#include "Nodes/InterchangeBaseNodeContainer.h"
#include "ObjectTools.h"

namespace
{
	FString Compact(const FString& Value)
	{
		FString Result;
		for (const TCHAR C : Value) if (FChar::IsAlnum(C)) Result.AppendChar(FChar::ToLower(C));
		return Result;
	}

	FString SourceName(FString Name)
	{
		Name = ObjectTools::SanitizeObjectName(Name);
		Name.RemoveFromStart(TEXT("MI_"));
		Name.RemoveFromStart(TEXT("M_"));
		return Name;
	}

	EOmniMaterialRole MaterialRole(const FString& Name)
	{
		if (Name.Contains(TEXT("Outline"))) return EOmniMaterialRole::Outline;
		if (Name.Contains(TEXT("Face"))) return EOmniMaterialRole::Face;
		return EOmniMaterialRole::Body;
	}
}

FString UOmniCharacterImportPipeline::SourceTextureGroup(const FString& Label)
{
	// Shader role and texture group are independent: Hair uses Body's shader, Hair's maps.
	for (const FString Group : {TEXT("EyeStar"), TEXT("Hair"), TEXT("Face"), TEXT("Body"), TEXT("Eye"), TEXT("Brow"), TEXT("Mouth"), TEXT("Glasses")})
		if (Label.Contains(Group)) return Group;
	return FString();
}

FString UOmniCharacterImportPipeline::MaterialTextureGroup(const FOmniKeywordMaterialBinding& Section, FName Parameter)
{
	if (Parameter == TEXT("Tex_Ramp"))
	{
		if (Section.TextureGroup == TEXT("Hair")) return TEXT("Hair");
		if (Section.ResolvedRole == EOmniMaterialRole::Face || Section.ResolvedRole == EOmniMaterialRole::Body) return TEXT("Body");
	}
	return Section.TextureGroup;
}

TSet<FName> UOmniCharacterImportPipeline::TextureParametersFromKeywords(const FString& Label) const
{
	const FString Name = Compact(Label);
	TSet<FName> Result;
	int32 BestLength = 0;
	for (const FOmniTextureKeywordRule& Rule : TextureKeywords)
	{
		for (const FString& Keyword : Rule.Keywords)
		{
			const FString Key = Compact(Keyword);
			if (Key.IsEmpty() || !Name.Contains(Key)) continue;
			// FaceLightMap is SDF, ShadowRamp is Ramp. The most specific alias wins.
			if (Key.Len() > BestLength) { Result.Reset(); BestLength = Key.Len(); }
			if (Key.Len() == BestLength) Result.Add(Rule.Parameter);
		}
	}
	return Result;
}

void UOmniCharacterImportPipeline::RefreshMaterialSourceData(UInterchangeBaseNodeContainer* Container)
{
	if (bUseKeywordMaterials)
	{
		FaceSourceContainer = Container;
		FaceSourceUid.Reset();
		RefreshKeywordMaterials(Container);
	}
	else
	{
		RefreshFaceSourceData(Container);
		RefreshBodySourceData(Container);
	}
}

void UOmniCharacterImportPipeline::RefreshKeywordMaterials(UInterchangeBaseNodeContainer* Container)
{
	KeywordMaterialError.Reset();
	if (!Container) { MaterialMappingStatus = TEXT("选择 FBX 后，按 Outline → Face → Body 匹配母材质；未命中使用 Body。"); return; }
	TArray<UInterchangeBaseNode*> Sources;
	Container->GetNodesOfType(Sources);
	Sources.RemoveAll([](const UInterchangeBaseNode* Node) { return !Node->IsA<UInterchangeShaderGraphNode>() && !Node->IsA<UInterchangeMaterialInstanceNode>(); });
	if (Sources.IsEmpty()) return; // Initial context setup runs before translation.
	Sources.Sort([](const UInterchangeBaseNode& A, const UInterchangeBaseNode& B) { return A.GetUniqueID() < B.GetUniqueID(); });
	// Strip only a shared source prefix, independently of the user-supplied destination name.
	FString CommonPrefix = SourceName(Sources[0]->GetDisplayLabel());
	for (const UInterchangeBaseNode* Node : Sources)
	{
		const FString Name = SourceName(Node->GetDisplayLabel());
		int32 Count = 0;
		while (Count < CommonPrefix.Len() && Count < Name.Len() && FChar::ToLower(CommonPrefix[Count]) == FChar::ToLower(Name[Count])) ++Count;
		CommonPrefix.LeftInline(Count);
	}
	int32 Separator;
	if (Sources.Num() < 2 || !CommonPrefix.FindLastChar(TEXT('_'), Separator)) CommonPrefix.Reset();
	else CommonPrefix.LeftInline(Separator + 1);
	TArray<FOmniKeywordMaterialBinding> Rows;
	for (const UInterchangeBaseNode* Source : Sources)
	{
		FOmniKeywordMaterialBinding Row;
		if (const auto* Previous = MaterialMappings.FindByPredicate([Source](const FOmniKeywordMaterialBinding& Item)
		{
			return Item.SourceUid == Source->GetUniqueID() || (Item.SourceUid.IsEmpty() && Item.SourceMaterial == Source->GetDisplayLabel());
		})) Row = *Previous;
		Row.SourceUid = Source->GetUniqueID();
		Row.SourceMaterial = Source->GetDisplayLabel();
		Row.AssetSuffix = SourceName(Row.SourceMaterial);
		if (!CommonPrefix.IsEmpty()) Row.AssetSuffix.RemoveFromStart(CommonPrefix);
		if (Row.AssetSuffix.IsEmpty()) Row.AssetSuffix = SourceName(Row.SourceMaterial);
		Row.ResolvedRole = Row.Role == EOmniMaterialRole::Auto ? MaterialRole(Row.SourceMaterial) : Row.Role;
		Row.TextureGroup = SourceTextureGroup(Row.SourceMaterial);
		Row.Parent = Row.ResolvedRole == EOmniMaterialRole::Outline ? OutlineParentMaterial : (Row.ResolvedRole == EOmniMaterialRole::Face ? FaceParentMaterial : BodyParentMaterial);
		TSet<FName> PreviousParameters;
		for (const auto& Binding : Row.Textures) PreviousParameters.Add(Binding.Parameter);
		Omni::CharacterImport::RefreshMaterialTextureParameters(Row.Parent.LoadSynchronous(), Row.Textures);
		for (auto& Binding : Row.Textures) if (!PreviousParameters.Contains(Binding.Parameter)) Binding.Mode = EOmniFaceTextureMode::AutoMatch;
		FString Error;
		if (!Row.Parent.Get()) Error = TEXT("请选择有效的母材质。");
		else ResolveKeywordTextures(Container, Source, Row, Error);
		Row.Status = Error.IsEmpty() ? TEXT("已按关键词匹配；缺失纹理沿用母材质默认值。") : Error;
		if (!Error.IsEmpty()) KeywordMaterialError = Row.SourceMaterial + TEXT("：") + Error;
		Rows.Add(MoveTemp(Row));
	}
	MaterialMappings = MoveTemp(Rows);
	if (MaterialMappings.ContainsByPredicate([](const auto& Row) { return Row.ResolvedRole == EOmniMaterialRole::Body; }) && !ResolveBodyToonProfileTemplate())
		KeywordMaterialError = TEXT("Body 母材质没有唯一的 Toon Profile，请指定 Body Profile 模板。");
	MaterialMappingStatus = KeywordMaterialError.IsEmpty()
		? FString::Printf(TEXT("%d 个源材质各建一个 MI；Outline 优先，Face 次之，其余使用 Body。"), MaterialMappings.Num()) : KeywordMaterialError;
	if (bIsReimportContext && !bUpdateKeywordMaterialsOnReimport) MaterialMappingStatus = TEXT("重导入保留已有材质和纹理；需要重新匹配时勾选更新。");
}

void UOmniCharacterImportPipeline::ResolveKeywordTextures(const UInterchangeBaseNodeContainer* Container,
	const UInterchangeBaseNode* SourceMaterial, FOmniKeywordMaterialBinding& Section, FString& Error) const
{
	TArray<UInterchangeTextureNode*> Textures;
	Container->GetNodesOfType(Textures);
	for (FOmniFaceTextureBinding& Row : Section.Textures)
	{
		TArray<FOmniFaceTextureBinding> One{Row};
		FString RowError;
		// Preserve explicit selections, and prefer actual source links over misleading filenames.
		Omni::CharacterImport::ResolveMaterialTextures(Container, SourceMaterial, TEXT("TextureNaming"), One, RowError);
		Row = MoveTemp(One[0]);
		if (Row.Mode != EOmniFaceTextureMode::AutoMatch || !Row.MatchPattern.IsEmpty() || !Row.ResolvedTextureUid.IsEmpty() || !RowError.IsEmpty())
		{
			if (!RowError.IsEmpty()) Error = RowError;
			continue;
		}
		const FString MatchGroup = MaterialTextureGroup(Section, Row.Parameter);
		TArray<UInterchangeTextureNode*> Scoped;
		TArray<UInterchangeTextureNode*> Unscoped;
		for (UInterchangeTextureNode* Texture : Textures)
		{
			const TSet<FName> Channels = TextureParametersFromKeywords(Texture->GetDisplayLabel());
			const bool bKnownParameter = TextureKeywords.ContainsByPredicate([&Row](const auto& Rule) { return Rule.Parameter == Row.Parameter; });
			const bool bMatch = Channels.Contains(Row.Parameter) || (!bKnownParameter && Channels.IsEmpty() && Compact(Texture->GetDisplayLabel()).Contains(Compact(Row.Parameter.ToString().RightChop(4))));
			if (!bMatch) continue;
			const FString Group = SourceTextureGroup(Texture->GetDisplayLabel());
			if (!MatchGroup.IsEmpty() && Group == MatchGroup) Scoped.Add(Texture);
			else if (Group.IsEmpty()) Unscoped.Add(Texture);
		}
		// Ramp must stay in its designated Body/Hair group; absence inherits the parent.
		auto& Matches = Row.Parameter == TEXT("Tex_Ramp") ? Scoped : (Scoped.IsEmpty() ? Unscoped : Scoped);
		Matches.Sort([](const UInterchangeTextureNode& A, const UInterchangeTextureNode& B)
		{
			const int32 Order = A.GetDisplayLabel().Compare(B.GetDisplayLabel(), ESearchCase::IgnoreCase);
			return Order == 0 ? A.GetUniqueID() < B.GetUniqueID() : Order < 0;
		});
		if (!Matches.IsEmpty())
		{
			Row.ResolvedTextureUid = Matches[0]->GetUniqueID();
			Row.bUseParentDefault = false;
			Row.Status = Matches.Num() == 1 ? TEXT("关键词匹配：") : FString::Printf(TEXT("%d 张候选，按源名称排序自动选择第一张："), Matches.Num());
			Row.Status += Matches[0]->GetDisplayLabel();
		}
	}
}

bool UOmniCharacterImportPipeline::ValidateKeywordMaterials(TOptional<FText>& OutInvalidReason) const
{
	if (!bUseKeywordMaterials || (bIsReimportContext && !bUpdateKeywordMaterialsOnReimport) || KeywordMaterialError.IsEmpty()) return true;
	OutInvalidReason = FText::FromString(KeywordMaterialError);
	return false;
}

bool UOmniCharacterImportPipeline::PrepareKeywordMaterials(UInterchangeBaseNodeContainer* Container)
{
	if (!bIsReimportContext && !Omni::CharacterImport::PrepareTextureSettings(Container, MaterialMappings,
		{[this](const FString& Message, bool bError) { Report(Message, bError); }})) return false;
	TSet<FString> Names;
	for (int32 Index = 0; Index < MaterialMappings.Num(); ++Index)
	{
		const auto& Row = MaterialMappings[Index];
		const FString Key = UInterchangeFactoryBaseNode::BuildFactoryNodeUid(Row.SourceUid);
		KeywordFactoryRows.Add(Key, Index);
		if (bIsReimportContext && !bUpdateKeywordMaterialsOnReimport) continue;
		if (!Cast<UInterchangeMaterialInstanceFactoryNode>(Container->GetFactoryNode(Key)))
		{
			Report(TEXT("Enable Import Materials and Material Instances in the preceding FBX pipeline: ") + Row.SourceMaterial, true);
			return false;
		}
		if (!Omni::CharacterImport::PrepareMaterialTextures(Container, Key, Row.Parent, Row.Textures, {[this](const FString& Message, bool bError) { Report(Message, bError); }})) return false;
		if (bIsReimportContext) continue;
		const FString Name = TEXT("MI_") + ResolvedName + TEXT("_") + Row.AssetSuffix;
		const FString Package = CharacterRoot / ActiveProfile->MaterialFolder / Name;
		if (Names.Contains(Name.ToLower()) || !FPackageName::IsValidLongPackageName(Package) || FPackageName::DoesPackageExist(Package) || StaticFindObject(UObject::StaticClass(), nullptr, *(Package + TEXT(".") + Name)))
		{
			Report(TEXT("Material name is duplicated or destination is occupied: ") + Package, true);
			return false;
		}
		Names.Add(Name.ToLower());
		Container->GetFactoryNode(Key)->SetAssetName(Name);
	}
	return true;
}

void UOmniCharacterImportPipeline::ApplyKeywordMaterial(const UInterchangeBaseNodeContainer* Container,
	const FString& NodeKey, UMaterialInstanceConstant* Material, bool bReimport)
{
	if ((bReimport || bIsReimportContext) && !bUpdateKeywordMaterialsOnReimport) return;
	const int32* Index = KeywordFactoryRows.Find(NodeKey);
	if (!Index || !MaterialMappings.IsValidIndex(*Index)) return;
	const auto& Row = MaterialMappings[*Index];
	if (Row.ResolvedRole != EOmniMaterialRole::Body) Omni::CharacterImport::ClearOwnedToonProfile(Material);
	Omni::CharacterImport::ApplyMaterialTextures(Container, NodeKey, Material, Row.Parent.LoadSynchronous(), Row.Textures, {[this](const FString& Message, bool bError) { Report(Message, bError); }});
}
