// Copyright © 2026 张鸿源. All Rights Reserved.
#include "Import/Assets/OmniImportAssetLayout.h"
#include "Import/OmniCharacterImportProfile.h"
#include "InterchangeAnimSequenceFactoryNode.h"
#include "InterchangeMaterialFactoryNode.h"
#include "InterchangePhysicsAssetFactoryNode.h"
#include "InterchangeSkeletalMeshFactoryNode.h"
#include "InterchangeSkeletonFactoryNode.h"
#include "InterchangeStaticMeshFactoryNode.h"
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
	const UOmniCharacterImportProfile& Profile, const FString& ResolvedName, const bool bNormalizeTextureNames)
{
	TArray<UInterchangeFactoryBaseNode*> Nodes;
	Container->GetNodesOfType(Nodes);
	Nodes.Sort([](const UInterchangeFactoryBaseNode& A, const UInterchangeFactoryBaseNode& B)
	{
		return A.GetUniqueID() < B.GetUniqueID();
	});
	const int32 MeshCount = Nodes.FilterByPredicate([](const UInterchangeFactoryBaseNode* Node)
	{
		return Node->IsA<UInterchangeSkeletalMeshFactoryNode>() || Node->IsA<UInterchangeStaticMeshFactoryNode>();
	}).Num();
	TMap<FString, FString> MeshNames;
	TSet<FString> UsedMeshNames;
	for (UInterchangeFactoryBaseNode* Node : Nodes)
	{
		const bool bSkeletal = Node->IsA<UInterchangeSkeletalMeshFactoryNode>();
		if (!bSkeletal && !Node->IsA<UInterchangeStaticMeshFactoryNode>()) continue;
		const FString Prefix = bSkeletal ? TEXT("SKM_") : TEXT("SM_");
		FString Base = MeshCount == 1 ? ResolvedName : ObjectTools::SanitizeObjectName(Node->GetAssetName());
		if (Base.IsEmpty()) Base = ResolvedName;
		const FString InitialName = WithPrefix(Base, Prefix);
		FString Name = InitialName;
		for (int32 Suffix = 2; UsedMeshNames.Contains(Name.ToLower()); ++Suffix)
		{
			Name = InitialName + FString::Printf(TEXT("_%02d"), Suffix);
		}
		UsedMeshNames.Add(Name.ToLower());
		MeshNames.Add(Node->GetUniqueID(), Name);
	}
	auto MeshBaseName = [&MeshNames](const FString& Uid)
	{
		FString Name = MeshNames.FindRef(Uid);
		Name.RemoveFromStart(TEXT("SKM_"));
		Name.RemoveFromStart(TEXT("SM_"));
		return Name;
	};
	TSet<FString> UsedDestinations;
	for (UInterchangeFactoryBaseNode* Node : Nodes)
	{
		FString Folder;
		FString Prefix;
		FString Name = ObjectTools::SanitizeObjectName(Node->GetAssetName());
		bool bManaged = true;
		if (const FString* MeshName = MeshNames.Find(Node->GetUniqueID()))
		{
			Folder = Profile.MeshFolder;
			Name = *MeshName;
		}
		else if (const UInterchangeSkeletonFactoryNode* Skeleton = Cast<UInterchangeSkeletonFactoryNode>(Node))
		{
			Folder = Profile.MeshFolder;
			Prefix = TEXT("SK_");
			TArray<FString> MeshUids;
			Skeleton->GetCustomSkeletalMeshFactoryNodeUids(MeshUids);
			MeshUids.Sort();
			// A shared Skeleton is named after its first mesh, but remains shared.
			for (const FString& Uid : MeshUids)
			{
				const FString OwnerName = MeshBaseName(Uid);
				if (!OwnerName.IsEmpty()) { Name = OwnerName; break; }
			}
		}
		else if (const UInterchangePhysicsAssetFactoryNode* Physics = Cast<UInterchangePhysicsAssetFactoryNode>(Node))
		{
			Folder = Profile.PhysicsFolder;
			Prefix = TEXT("PHYS_");
			FString MeshUid;
			if (Physics->GetCustomSkeletalMeshUid(MeshUid))
			{
				const FString OwnerName = MeshBaseName(MeshUid);
				if (!OwnerName.IsEmpty()) Name = OwnerName;
			}
		}
		else if (Node->IsA<UInterchangeTextureFactoryNode>())
		{
			Folder = Profile.TextureFolder;
			if (bNormalizeTextureNames) Prefix = TEXT("T_");
		}
		else if (Node->IsA<UInterchangeBaseMaterialFactoryNode>())
		{
			Folder = Profile.MaterialFolder;
			Prefix = Node->IsA<UInterchangeMaterialInstanceFactoryNode>() ? TEXT("MI_") : TEXT("M_");
		}
		else if (Node->IsA<UInterchangeAnimSequenceFactoryNode>())
		{
			Folder = Profile.AnimationFolder;
		}
		else bManaged = false;
		if (bManaged)
		{
			const FString InitialName = WithPrefix(Name, Prefix);
			Name = InitialName;
			for (int32 Suffix = 2; UsedDestinations.Contains((Folder / Name).ToLower()); ++Suffix)
			{
				Name = InitialName + FString::Printf(TEXT("_%02d"), Suffix);
			}
			UsedDestinations.Add((Folder / Name).ToLower());
			Node->SetCustomSubPath(Profile.bCreateCharacterFolder ? ResolvedName / Folder : Folder);
			Node->SetAssetName(Name);
		}
	}
}
