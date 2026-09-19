// Copyright © 2026 张鸿源. All Rights Reserved.
#include "Import/Assets/OmniImportPostProcessAnim.h"
#include "AssetToolsModule.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimInstance.h"
#include "AnimationGraphSchema.h"
#include "AnimGraphNode_LinkedInputPose.h"
#include "AnimGraphNode_Root.h"
#include "EdGraph/EdGraph.h"
#include "Engine/SkeletalMesh.h"
#include "Factories/AnimBlueprintFactory.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/PackageName.h"

bool Omni::CharacterImport::EnsurePostProcessAnimationBlueprint(USkeletalMesh* Mesh, const FImportReporter& Report)
{
	if (!Mesh || !Mesh->GetSkeleton())
	{
		Report(TEXT("Post-process blueprint requires a skeletal mesh with a skeleton."), true);
		return false;
	}
	if (Mesh->GetPostProcessAnimBlueprint())
	{
		Report(TEXT("Retained existing post-process animation blueprint: ") + Mesh->GetPostProcessAnimBlueprint()->GetPathName());
		return true;
	}

	// Use the actual mesh package, including on reimport or after a user moves it.
	const FString Folder = FPackageName::GetLongPackagePath(Mesh->GetOutermost()->GetName());
	FString MeshName = Mesh->GetName();
	MeshName.RemoveFromStart(TEXT("SKM_"));
	const FString Name = TEXT("ABP_") + MeshName + TEXT("_Post");
	const FString PackageName = Folder / Name;
	const FString ObjectPath = PackageName + TEXT(".") + Name;
	if (!FPackageName::IsValidLongPackageName(PackageName) || Mesh->GetOutermost() == GetTransientPackage())
	{
		Report(TEXT("Post-process blueprint needs a persistent mesh destination: ") + PackageName, true);
		return false;
	}
	UObject* Existing = FindObject<UObject>(nullptr, *ObjectPath);
	const bool bPackageExists = FPackageName::DoesPackageExist(PackageName);
	if (!Existing && bPackageExists) Existing = LoadObject<UObject>(nullptr, *ObjectPath);
	UAnimBlueprint* Blueprint = Cast<UAnimBlueprint>(Existing);
	if ((Existing || bPackageExists) && !Blueprint)
	{
		Report(TEXT("Post-process destination occupied or unreadable; left unchanged: ") + ObjectPath, true);
		return false;
	}
	if (!Blueprint)
	{
		UAnimBlueprintFactory* Factory = NewObject<UAnimBlueprintFactory>();
		Factory->TargetSkeleton = Mesh->GetSkeleton();
		Factory->PreviewSkeletalMesh = Mesh;
		Factory->ParentClass = UAnimInstance::StaticClass();
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();
		Blueprint = Cast<UAnimBlueprint>(AssetTools.CreateAsset(Name, Folder, UAnimBlueprint::StaticClass(), Factory));
		if (!Blueprint)
		{
			Report(TEXT("Could not create post-process blueprint: ") + ObjectPath, true);
			return false;
		}
		UEdGraph* AnimGraph = nullptr;
		for (UEdGraph* Graph : Blueprint->FunctionGraphs)
		{
			if (Graph && Graph->GetFName() == UEdGraphSchema_K2::GN_AnimGraph)
			{
				AnimGraph = Graph;
				break;
			}
		}
		TArray<UAnimGraphNode_Root*> Roots;
		if (AnimGraph) AnimGraph->GetNodesOfClass(Roots);
		if (Roots.Num() != 1)
		{
			Report(TEXT("New post-process blueprint has no unique AnimGraph output; left unassigned: ") + ObjectPath, true);
			return false;
		}
		FGraphNodeCreator<UAnimGraphNode_LinkedInputPose> Creator(*AnimGraph);
		UAnimGraphNode_LinkedInputPose* Input = Creator.CreateNode();
		Input->NodePosX = Roots[0]->NodePosX - 300;
		Input->NodePosY = Roots[0]->NodePosY;
		Creator.Finalize();
		UEdGraphPin* OutputPin = nullptr;
		UEdGraphPin* ResultPin = nullptr;
		for (UEdGraphPin* Pin : Input->Pins)
		{
			if (Pin->Direction == EGPD_Output && UAnimationGraphSchema::IsPosePin(Pin->PinType)) OutputPin = Pin;
		}
		for (UEdGraphPin* Pin : Roots[0]->Pins)
		{
			if (Pin->Direction == EGPD_Input && UAnimationGraphSchema::IsPosePin(Pin->PinType)) ResultPin = Pin;
		}
		if (!OutputPin || !ResultPin || !AnimGraph->GetSchema()->TryCreateConnection(OutputPin, ResultPin))
		{
			Report(TEXT("Could not connect post-process input pose; left unassigned: ") + ObjectPath, true);
			return false;
		}
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
		FKismetEditorUtilities::CompileBlueprint(Blueprint);
		Report(TEXT("Created pose-passthrough post-process blueprint: ") + ObjectPath);
	}
	// Reuse existing graphs as-is. Incompatible or uncompiled assets require user review.
	if (Blueprint->TargetSkeleton != Mesh->GetSkeleton() || !Blueprint->GeneratedClass ||
		!Blueprint->GeneratedClass->IsChildOf(UAnimInstance::StaticClass()) ||
		(Blueprint->Status != BS_UpToDate && Blueprint->Status != BS_UpToDateWithWarnings))
	{
		Report(TEXT("Post-process blueprint has a different skeleton or needs a successful compile; left unassigned: ") + ObjectPath, true);
		return false;
	}
	Mesh->Modify();
	Mesh->SetPostProcessAnimBlueprint(TSubclassOf<UAnimInstance>(Blueprint->GeneratedClass.Get()));
	FPropertyChangedEvent ChangedEvent(FindFProperty<FProperty>(USkeletalMesh::StaticClass(), USkeletalMesh::GetPostProcessAnimBlueprintMemberName()));
	Mesh->PostEditChangeProperty(ChangedEvent);
	(void)Mesh->MarkPackageDirty();
	Report(TEXT("Assigned post-process animation blueprint: ") + ObjectPath);
	return true;
}
