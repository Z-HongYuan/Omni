// Copyright © 2026 张鸿源. All Rights Reserved.
#include "Import/Materials/OmniImportMaterialApplier.h"
#include "Engine/Texture.h"
#include "InterchangeMaterialFactoryNode.h"
#include "InterchangeShaderGraphNode.h"
#include "MaterialEditingLibrary.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Nodes/InterchangeBaseNodeContainer.h"

bool Omni::CharacterImport::SetTextureParameterVerified(UMaterialInstanceConstant* Material, FName Parameter, UTexture* Texture)
{
	if (!Material || !Texture) return false;
	const FMaterialParameterInfo Info(Parameter);
	Material->SetTextureParameterValueEditorOnly(Info, Texture);
	UTexture* Applied = nullptr;
	return Material->GetTextureParameterValue(Info, Applied, true) && Applied == Texture;
}

bool Omni::CharacterImport::PrepareMaterialTextures(UInterchangeBaseNodeContainer* Container,
                                                           const FString& FactoryUid, const TSoftObjectPtr<UMaterialInterface>& Parent, const TArray<FOmniFaceTextureBinding>& Bindings, const FImportReporter& Report)
{
	UInterchangeMaterialInstanceFactoryNode* Factory = Cast<UInterchangeMaterialInstanceFactoryNode>(Container->GetFactoryNode(FactoryUid));
	if (!Factory) return false;
	Factory->SetCustomParent(Parent.ToSoftObjectPath().ToString());
	for (const FOmniFaceTextureBinding& Row : Bindings)
	{
		const FString Key = UInterchangeShaderPortsAPI::MakeInputParameterKey(Row.Parameter.ToString());
		Factory->RemoveAttribute(Key);
		if (!Row.ResolvedTextureUid.IsEmpty())
		{
			const FString TextureFactoryUid = UInterchangeFactoryBaseNode::BuildFactoryNodeUid(Row.ResolvedTextureUid);
			if (!Container->GetFactoryNode(TextureFactoryUid))
			{
				Report(TEXT("Material texture has no import factory. Enable Import Textures: ") + Row.Parameter.ToString(), true);
				return false;
			}
			Factory->AddStringAttribute(Key, TextureFactoryUid);
			Factory->AddFactoryDependencyUid(TextureFactoryUid);
		}
	}
	Report(TEXT("Configured material parent and Tex_ parameters: ") + Parent.GetAssetName());
	return true;
}

void Omni::CharacterImport::ApplyMaterialTextures(const UInterchangeBaseNodeContainer* Container,
                                                         const FString& NodeKey, UMaterialInstanceConstant* Material, UMaterialInterface* Parent,
                                                         const TArray<FOmniFaceTextureBinding>& Bindings, const FImportReporter& Report)
{
	if (!Parent) return;
	Material->SetParentEditorOnly(Parent);
	for (const FOmniFaceTextureBinding& Row : Bindings)
	{
		if (Row.Mode == EOmniFaceTextureMode::ParentDefault || Row.bUseParentDefault)
		{
			// Remove only this Tex_ override so later parent-default edits continue to propagate.
			Material->TextureParameterValues.RemoveAll([&](const FTextureParameterValue& Value)
			{
				return Value.ParameterInfo.Association == EMaterialParameterAssociation::GlobalParameter && Value.ParameterInfo.Name == Row.Parameter;
			});
			continue;
		}
		UTexture* Texture = Row.Mode == EOmniFaceTextureMode::ExistingTexture ? Row.Texture.LoadSynchronous() : nullptr;
		if (!Row.ResolvedTextureUid.IsEmpty() && Container)
		{
			const UInterchangeFactoryBaseNode* TextureNode = Container->GetFactoryNode(UInterchangeFactoryBaseNode::BuildFactoryNodeUid(Row.ResolvedTextureUid));
			FSoftObjectPath Path;
			if (TextureNode && TextureNode->GetCustomReferenceObject(Path)) Texture = Cast<UTexture>(Path.TryLoad());
		}
		if (!Texture)
		{
			Report(TEXT("Material texture is unavailable: ") + Row.Parameter.ToString() + TEXT(" on ") + NodeKey, true);
			continue;
		}
		// UE 5.8 MaterialEditingLibrary's texture setter always returns false.
		// Use the native setter and verify the stored override instead.
		if (!SetTextureParameterVerified(Material, Row.Parameter, Texture))
			Report(TEXT("Could not verify material texture: ") + Row.Parameter.ToString() + TEXT(" on ") + NodeKey, true);
	}
	UMaterialEditingLibrary::UpdateMaterialInstance(Material);
	(void)Material->MarkPackageDirty();
	Report(TEXT("Applied material textures: ") + Material->GetPathName());
}
