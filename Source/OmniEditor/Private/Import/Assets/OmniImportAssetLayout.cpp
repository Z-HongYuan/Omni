// Copyright © 2026 张鸿源. All Rights Reserved.
#include "Import/Assets/OmniImportAssetLayout.h"
#include "Import/OmniCharacterImportProfile.h"
#include "InterchangeMaterialFactoryNode.h"
#include "InterchangePhysicsAssetFactoryNode.h"
#include "InterchangeSkeletalMeshFactoryNode.h"
#include "InterchangeSkeletonFactoryNode.h"
#include "InterchangeTextureFactoryNode.h"
#include "Nodes/InterchangeBaseNodeContainer.h"
#include "ObjectTools.h"

namespace Omni::CharacterImport
{
	bool IsRelativeFolder(const FString& Folder)
	{
		if (Folder.IsEmpty()) return true; // Empty selects the character root.
		if (Folder.StartsWith(TEXT("/")) || Folder.EndsWith(TEXT("/"))) return false;
		TArray<FString> Parts;
		Folder.ParseIntoArray(Parts, TEXT("/"), false);
		return !Parts.ContainsByPredicate([](const FString& Part)
		{
			return Part.IsEmpty() || Part == TEXT(".") || Part == TEXT("..") || ObjectTools::SanitizeObjectName(Part) != Part;
		});
	}

	FString WithPrefix(const FString& Name, const FString& Prefix)
	{
		return Name.StartsWith(Prefix) ? Name : Prefix + Name;
	}
}

void Omni::CharacterImport::ApplyAssetLayout(UInterchangeBaseNodeContainer* Container,
	const UOmniCharacterImportProfile& Profile, const FString& ResolvedName)
{
	Container->IterateNodesOfType<UInterchangeFactoryBaseNode>([&](const FString&, UInterchangeFactoryBaseNode* Node)
	{
		FString Folder;
		FString Prefix;
		FString Name = ObjectTools::SanitizeObjectName(Node->GetAssetName());
		if (Node->IsA<UInterchangeSkeletalMeshFactoryNode>())
		{
			Folder = Profile.MeshFolder;
			Prefix = TEXT("SKM_");
			Name = ResolvedName;
		}
		else if (Node->IsA<UInterchangeSkeletonFactoryNode>())
		{
			Folder = FString(); // Keep the skeleton beside its mesh.
			Prefix = TEXT("SK_");
			Name = ResolvedName;
		}
		else if (Node->IsA<UInterchangePhysicsAssetFactoryNode>())
		{
			Folder = Profile.PhysicsFolder;
			Prefix = TEXT("PHYS_");
			Name = ResolvedName;
		}
		else if (Node->IsA<UInterchangeTextureFactoryNode>())
		{
			Folder = Profile.TextureFolder;
			Prefix = TEXT("T_");
		}
		else if (Node->IsA<UInterchangeBaseMaterialFactoryNode>())
		{
			Folder = Profile.MaterialFolder;
			Prefix = Node->IsA<UInterchangeMaterialInstanceFactoryNode>() ? TEXT("MI_") : TEXT("M_");
		}
		if (!Prefix.IsEmpty())
		{
			Node->SetCustomSubPath(Profile.bCreateCharacterFolder ? ResolvedName / Folder : Folder);
			Node->SetAssetName(WithPrefix(Name, Prefix));
		}
	});
}
