// Copyright © 2026 张鸿源. All Rights Reserved.
#include "Import/Materials/OmniImportTextureSettings.h"

#include "Engine/Texture.h"
#include "Import/OmniCharacterImportProfile.h"
#include "InterchangeTextureFactoryNode.h"
#include "Nodes/InterchangeBaseNodeContainer.h"

bool Omni::CharacterImport::PrepareTextureSettings(UInterchangeBaseNodeContainer* Container,
	const TArray<FOmniKeywordMaterialBinding>& Materials, const FImportReporter& Report)
{
	TMap<FString, EOmniImportTextureUsage> Usages;
	for (const FOmniKeywordMaterialBinding& Material : Materials)
	{
		for (const FOmniFaceTextureBinding& Binding : Material.Textures)
		{
			if (Binding.ResolvedTextureUid.IsEmpty() || Binding.Mode == EOmniFaceTextureMode::ExistingTexture) continue;
			EOmniImportTextureUsage Usage = EOmniImportTextureUsage::Preserve;
			if (Binding.Parameter == TEXT("Tex_BaseColor")) Usage = EOmniImportTextureUsage::Color;
			else if (Binding.Parameter == TEXT("Tex_Normal")) Usage = EOmniImportTextureUsage::Normal;
			else if (Binding.Parameter == TEXT("Tex_LightMap") || Binding.Parameter == TEXT("Tex_SDF") ||
				Binding.Parameter == TEXT("Tex_Shadow") || Binding.Parameter == TEXT("Tex_MetalMap")) Usage = EOmniImportTextureUsage::Mask;
			// Ramp 和未知参数没有统一色彩空间约定，保留原设置。
			if (Usage == EOmniImportTextureUsage::Preserve) continue;
			if (const EOmniImportTextureUsage* Previous = Usages.Find(Binding.ResolvedTextureUid); Previous && *Previous != Usage)
			{
				Report(TEXT("Source texture has incompatible color/data/normal uses: ") + Binding.ResolvedTextureUid, true);
				return false;
			}
			Usages.Add(Binding.ResolvedTextureUid, Usage);
		}
	}
	// 先验证全部用途是否冲突，再修改纹理工厂。
	for (const auto& Pair : Usages)
	{
		UInterchangeTextureFactoryNode* Node = Cast<UInterchangeTextureFactoryNode>(Container->GetFactoryNode(
			UInterchangeFactoryBaseNode::BuildFactoryNodeUid(Pair.Key)));
		if (!Node) continue; // 缺失工厂由 PrepareMaterialTextures 报告。
		Node->SetCustomSRGB(Pair.Value == EOmniImportTextureUsage::Color);
		// 遮罩数据关闭 sRGB，并使用 Default 压缩。
		Node->SetCustomCompressionSettings(Pair.Value == EOmniImportTextureUsage::Normal ? TC_Normalmap : TC_Default);
	}
	return true;
}
