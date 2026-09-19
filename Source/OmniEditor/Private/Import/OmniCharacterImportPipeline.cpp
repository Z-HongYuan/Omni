// Copyright © 2026 张鸿源. All Rights Reserved.
#include "Import/OmniCharacterImportPipeline.h"

#include "Import/OmniCharacterImportProfile.h"
#include "Import/Assets/OmniImportAssetLayout.h"
#include "Import/Assets/OmniImportPostProcessAnim.h"
#include "Import/Assets/OmniImportRigAssets.h"
#include "Engine/SkeletalMesh.h"
#include "InterchangeMaterialFactoryNode.h"
#include "InterchangeProjectSettings.h"
#include "InterchangeSkeletalMeshFactoryNode.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Nodes/InterchangeBaseNodeContainer.h"
#include "ObjectTools.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniCharacterImportPipeline)

DEFINE_LOG_CATEGORY_STATIC(LogOmniCharacterImport, Log, All);

bool UOmniCharacterImportPipeline::SaveProjectSettings()
{
	UInterchangeProjectSettings* Settings = GetMutableDefault<UInterchangeProjectSettings>();
	// SaveConfig updates GConfig as well as generated Engine.ini. Writing the file
	// externally leaves stale cached settings that can overwrite it on shutdown.
	Settings->SaveConfig();
	return Settings->TryUpdateDefaultConfigFile();
}

bool UOmniCharacterImportPipeline::IsSettingsAreValid(TOptional<FText>& OutInvalidReason) const
{
	const UOmniCharacterImportProfile* Settings = Profile.LoadSynchronous();
	if (!Settings)
	{
		OutInvalidReason = FText::FromString(TEXT("Choose an Omni Character Import Profile."));
		return false;
	}
	for (const FString& Folder : {Settings->MeshFolder, Settings->TextureFolder, Settings->MaterialFolder, Settings->RigFolder, Settings->PhysicsFolder})
	{
		if (!Omni::CharacterImport::IsRelativeFolder(Folder))
		{
			OutInvalidReason = FText::FromString(TEXT("Profile folders must be relative Content folder names without '..' or invalid characters."));
			return false;
		}
	}
	if (!CharacterName.IsEmpty() && (ObjectTools::SanitizeObjectName(CharacterName) != CharacterName || CharacterName.Contains(TEXT("/"))))
	{
		OutInvalidReason = FText::FromString(TEXT("Character Name must be a single valid asset name."));
		return false;
	}
	if (!Settings->SourceRigLabel.IsEmpty() &&
		(ObjectTools::SanitizeObjectName(Settings->SourceRigLabel) != Settings->SourceRigLabel || Settings->SourceRigLabel.Contains(TEXT("/"))))
	{
		OutInvalidReason = FText::FromString(TEXT("Source Rig Label must be a single valid asset name."));
		return false;
	}
	if (!bUseKeywordMaterials && !ValidateLegacyMaterialRules(Settings, OutInvalidReason)) return false;

	return (bUseKeywordMaterials ? ValidateKeywordMaterials(OutInvalidReason) : (ValidateFaceSettings(OutInvalidReason) && ValidateBodySettings(OutInvalidReason))) && ValidateTextureNames(OutInvalidReason) && Super::IsSettingsAreValid(OutInvalidReason);
}

void UOmniCharacterImportPipeline::Report(const FString& Message, bool bError)
{
	Messages.Add(Message);
	if (bError) { UE_LOG(LogOmniCharacterImport, Error, TEXT("%s"), *Message); }
	else { UE_LOG(LogOmniCharacterImport, Display, TEXT("%s"), *Message); }
}

void UOmniCharacterImportPipeline::ExecutePipeline(UInterchangeBaseNodeContainer* Container,
                                                   const TArray<UInterchangeSourceData*>& Sources, const FString& ContentBasePath)
{
	Messages.Reset();
	bPrepared = false;
	MaterialRuleIndices.Reset();
	ActiveProfile = Profile.LoadSynchronous();
	FaceFactoryKeys.Reset();
	BodyFactoryRows.Reset();
	KeywordFactoryRows.Reset();
	RefreshMaterialSourceData(Container);
	RefreshTextureNames(Container);
	TOptional<FText> InvalidReason;
	if (!Container || !IsSettingsAreValid(InvalidReason) || Sources.Num() != 1 || !Sources[0])
	{
		Report(InvalidReason.IsSet() ? InvalidReason->ToString() : TEXT("Expected one FBX source per import job."), true);
		if (Container) Container->IterateNodesOfType<UInterchangeFactoryBaseNode>([](const FString&, UInterchangeFactoryBaseNode* Node) { Node->SetEnabled(false); });
		return;
	}
	TArray<UInterchangeSkeletalMeshFactoryNode*> SkeletalNodes;
	Container->GetNodesOfType(SkeletalNodes);
	if (SkeletalNodes.Num() != 1)
	{
		Report(TEXT("Expected one Skeletal Mesh factory node. Put the default FBX assets pipeline first, combine the character meshes, and exclude independent rigs."), true);
		// Reject the job rather than silently producing unconfigured assets.
		Container->IterateNodesOfType<UInterchangeFactoryBaseNode>([](const FString&, UInterchangeFactoryBaseNode* Node) { Node->SetEnabled(false); });
		return;
	}
	// The preceding FBX pipeline has already applied the dialog's custom Asset Name.
	// Keep that resolved name instead of overwriting it with the source filename.
	ResolvedName = CharacterName.IsEmpty() ? SkeletalNodes[0]->GetAssetName() : CharacterName;
	if (ResolvedName.IsEmpty()) ResolvedName = FPaths::GetBaseFilename(Sources[0]->GetFilename());
	ResolvedName = ObjectTools::SanitizeObjectName(ResolvedName);
	CharacterRoot = ActiveProfile->bCreateCharacterFolder ? ContentBasePath / ResolvedName : ContentBasePath;
	if (!FPackageName::IsValidLongPackageName(CharacterRoot / TEXT("Validation")))
	{
		Report(TEXT("Invalid character destination: ") + CharacterRoot, true);
		Container->IterateNodesOfType<UInterchangeFactoryBaseNode>([](const FString&, UInterchangeFactoryBaseNode* Node) { Node->SetEnabled(false); });
		return;
	}
	SkeletalNodes[0]->SetCustomImportMorphTarget(true);
	bPrepared = true;
	// Geometry-only reimport must not change material routing, paths, or hand-tuned companions.
	if (!(bUseKeywordMaterials ? PrepareKeywordMaterials(Container) : (PrepareFaceMaterial(Container) && PrepareBodyMaterials(Container))))
	{
		Container->IterateNodesOfType<UInterchangeFactoryBaseNode>([](const FString&, UInterchangeFactoryBaseNode* Node) { Node->SetEnabled(false); });
		bPrepared = false;
		return;
	}
	if (bIsReimportContext && !bReapplyMaterialRulesOnReimport)
	{
		Report(TEXT("Reimport: preserved material rules, paths and companion assets."));
		return;
	}
	if (!bIsReimportContext) Omni::CharacterImport::ApplyAssetLayout(Container, *ActiveProfile, ResolvedName);
	RefreshTextureNames(Container, ResolvedName);
	if (!PrepareTextureNames(Container))
	{
		Container->IterateNodesOfType<UInterchangeFactoryBaseNode>([](const FString&, UInterchangeFactoryBaseNode* Node) { Node->SetEnabled(false); });
		bPrepared = false;
		return;
	}
	if (bUseKeywordMaterials || !ActiveProfile->bConfigureMaterials)
	{
		Report(TEXT("Material parents and texture bindings use the dialog configuration."));
		return;
	}
	PrepareLegacyMaterials(Container);
}

void UOmniCharacterImportPipeline::ExecutePostImportPipeline(const UInterchangeBaseNodeContainer* Container,
                                                             const FString& NodeKey, UObject* CreatedAsset, bool bIsAReimport)
{
	if (!bPrepared || !ActiveProfile || !CreatedAsset) return;
	if (UMaterialInstanceConstant* Material = Cast<UMaterialInstanceConstant>(CreatedAsset))
	{
		if (bUseKeywordMaterials)
		{
			ApplyKeywordMaterial(Container, NodeKey, Material, bIsAReimport);
			return;
		}
		if (FaceFactoryKeys.Contains(NodeKey))
		{
			ApplyFaceMaterial(Container, NodeKey, Material, bIsAReimport);
			return;
		}
		const int32* RuleIndex = MaterialRuleIndices.Find(NodeKey);
		if (BodyFactoryRows.Contains(NodeKey))
		{
			ApplyBodyMaterial(Container, NodeKey, Material, bIsAReimport);
			return;
		}
		if (!RuleIndex || (bIsAReimport && !bReapplyMaterialRulesOnReimport)) return;
		ApplyLegacyMaterial(Container, NodeKey, Material, *RuleIndex);
	}
	if (USkeletalMesh* Mesh = Cast<USkeletalMesh>(CreatedAsset))
	{
		if (bIsAReimport) ApplyRetainedMaterials(Container, Mesh);
		ApplyBodyToonProfile(Container, Mesh, bIsAReimport);
		const FReferenceSkeleton& Skeleton = Mesh->GetRefSkeleton();
		Report(FString::Printf(TEXT("%s: %d bones, %d morph targets, %d material slots."), *Mesh->GetName(), Skeleton.GetNum(), Mesh->GetMorphTargets().Num(), Mesh->GetMaterials().Num()));
		if (!bIsAReimport) Omni::CharacterImport::CreateRigAssets(Mesh, *ActiveProfile, CharacterRoot, ResolvedName,
			{[this](const FString& Message, bool bError) { Report(Message, bError); }});
		else Report(TEXT("Retained existing IK Rig, retargeter and physics tuning on reimport."));
		if (ActiveProfile->bCreatePostProcessAnimBlueprint) EnsurePostProcessAnimationBlueprint(Mesh);
		const FString ReportFolder = FPaths::ProjectSavedDir() / TEXT("OmniCharacterImport");
		IFileManager::Get().MakeDirectory(*ReportFolder, true);
		FFileHelper::SaveStringArrayToFile(Messages, *(ReportFolder / (ResolvedName + TEXT(".log"))));
	}
}

bool UOmniCharacterImportPipeline::EnsurePostProcessAnimationBlueprint(USkeletalMesh* Mesh)
{
	return Omni::CharacterImport::EnsurePostProcessAnimationBlueprint(Mesh,
		{[this](const FString& Message, bool bError) { Report(Message, bError); }});
}
