// Copyright © 2026 张鸿源. All Rights Reserved.
#include "Import/Assets/OmniImportRigAssets.h"
#include "Import/OmniCharacterImportProfile.h"
#include "AssetToolsModule.h"
#include "Engine/SkeletalMesh.h"
#include "Misc/PackageName.h"
#include "RetargetEditor/IKRetargetFactory.h"
#include "RetargetEditor/IKRetargeterController.h"
#include "Retargeter/IKRetargeter.h"
#include "Retargeter/IKRetargetChainMapping.h"
#include "Retargeter/IKRetargetSettings.h"
#include "Rig/IKRigDefinition.h"
#include "Rig/Solvers/IKRigFullBodyIK.h"
#include "RigEditor/IKRigController.h"
#include "RigEditor/IKRigDefinitionFactory.h"

namespace
{
	void ConfigureAutoFBIK(UIKRigController& Controller, const USkeletalMesh& Mesh, FName RetargetRoot, const Omni::CharacterImport::FImportReporter& Report)
	{
		// These are the limb labels used by UE's auto-characterization and FBIK templates.
		// Requiring all four complete limbs prevents weapon/accessory rigs from getting a body solver.
		const TArray<FName> LimbNames{TEXT("LeftArm"), TEXT("RightArm"), TEXT("LeftLeg"), TEXT("RightLeg")};
		const FReferenceSkeleton& Skeleton = Mesh.GetRefSkeleton();
		const int32 RootIndex = Skeleton.FindBoneIndex(RetargetRoot);
		for (FName LimbName : LimbNames)
		{
			const FBoneChain* Chain = Controller.GetRetargetChainByName(LimbName);
			TArray<int32> Bones;
			if (!Chain || !Controller.GetIKRigSkeleton().ValidateChainAndGetBones(*Chain, Bones) || Bones.Num() < 3)
			{
				Report(TEXT("Auto FBIK skipped: mesh has no complete biped limb chain: ") + LimbName.ToString());
				return;
			}
			int32 Ancestor = Bones[0];
			while (Ancestor != INDEX_NONE && Ancestor != RootIndex) Ancestor = Skeleton.GetParentIndex(Ancestor);
			if (RootIndex == INDEX_NONE || Ancestor != RootIndex)
			{
				Report(TEXT("Auto FBIK skipped: retarget root is not an ancestor of limb: ") + LimbName.ToString());
				return;
			}
		}

		const int32 SolverIndex = Controller.GetNumSolvers();
		TSet<FName> PreviousGoals;
		for (const UIKRigEffectorGoal* Goal : Controller.GetAllGoals()) PreviousGoals.Add(Goal->GoalName);
		TSet<FName> PreviousExcludedBones;
		for (const FMeshBoneInfo& Bone : Skeleton.GetRefBoneInfo())
		{
			if (Controller.GetBoneExcluded(Bone.Name)) PreviousExcludedBones.Add(Bone.Name);
		}
		const bool bApplied = Controller.ApplyAutoFBIK();
		const FInstancedStruct* SolverStruct = Controller.GetSolverStructAtIndex(SolverIndex);
		const FIKRigFullBodyIKSolver* Solver = SolverStruct ? SolverStruct->GetPtr<FIKRigFullBodyIKSolver>() : nullptr;
		bool bValid = bApplied && Controller.GetNumSolvers() == SolverIndex + 1 && Solver && Solver->IsEnabled();
		// The profile's validated root takes precedence over the template's pelvis selection.
		bValid = bValid && Controller.SetStartBone(RetargetRoot, SolverIndex) && Controller.GetStartBone(SolverIndex) == RetargetRoot;
		for (FName LimbName : LimbNames)
		{
			const FBoneChain* Chain = Controller.GetRetargetChainByName(LimbName);
			const UIKRigEffectorGoal* Goal = Chain ? Controller.GetGoal(Chain->IKGoalName) : nullptr;
			bValid = bValid && Goal && Goal->BoneName == Chain->EndBone.BoneName && Controller.IsGoalConnectedToSolver(Goal->GoalName, SolverIndex);
		}
		if (!bValid)
		{
			// Roll back only this attempt; do not leave an incomplete solver/goal setup behind.
			while (Controller.GetNumSolvers() > SolverIndex) Controller.RemoveSolver(Controller.GetNumSolvers() - 1);
			TArray<FName> AddedGoals;
			for (const UIKRigEffectorGoal* Goal : Controller.GetAllGoals())
			{
				if (!PreviousGoals.Contains(Goal->GoalName)) AddedGoals.Add(Goal->GoalName);
			}
			for (FName GoalName : AddedGoals) Controller.RemoveGoal(GoalName);
			for (const FMeshBoneInfo& Bone : Skeleton.GetRefBoneInfo()) Controller.SetBoneExcluded(Bone.Name, PreviousExcludedBones.Contains(Bone.Name));
			Report(TEXT("Auto FBIK could not create a complete four-limb solver. The mesh must match a UE auto-characterization template; review the IK Rig."), true);
			return;
		}
		Report(TEXT("Configured Full Body IK with four limb goals and root: ") + RetargetRoot.ToString());
	}
}

void Omni::CharacterImport::CreateRigAssets(USkeletalMesh* Mesh, const UOmniCharacterImportProfile& Profile,
	const FString& CharacterRoot, const FString& ResolvedName, const FImportReporter& Report)
{
	if (!Profile.bCreateIKRig) return;
	const FReferenceSkeleton& Skeleton = Mesh->GetRefSkeleton();
	if (Skeleton.FindBoneIndex(Profile.RetargetRoot) == INDEX_NONE)
	{
		Report(TEXT("IK Rig skipped for mesh without retarget root: ") + Profile.RetargetRoot.ToString());
		return;
	}
	for (const FOmniImportRetargetChain& Chain : Profile.RetargetChains)
	{
		const int32 Start = Skeleton.FindBoneIndex(Chain.StartBone);
		const int32 End = Skeleton.FindBoneIndex(Chain.EndBone);
		int32 Ancestor = End;
		while (Ancestor != INDEX_NONE && Ancestor != Start) Ancestor = Skeleton.GetParentIndex(Ancestor);
		if (Chain.ChainName.IsNone() || Start == INDEX_NONE || End == INDEX_NONE || Ancestor != Start)
		{
			Report(TEXT("IK Rig skipped: invalid retarget chain: ") + Chain.ChainName.ToString(), true);
			return;
		}
	}
	const FString Folder = CharacterRoot / Profile.RigFolder;
	const FString RigName = TEXT("IK_") + ResolvedName;
	const FString RigPath = Folder / RigName + TEXT(".") + RigName;
	UIKRigDefinition* TargetRig = LoadObject<UIKRigDefinition>(nullptr, *RigPath, nullptr, LOAD_NoWarn);
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();
	if (!TargetRig)
	{
		if (FPackageName::DoesPackageExist(Folder / RigName))
		{
			Report(TEXT("IK destination occupied by another asset: ") + RigPath, true);
			return;
		}
		TargetRig = Cast<UIKRigDefinition>(AssetTools.CreateAsset(RigName, Folder, UIKRigDefinition::StaticClass(), NewObject<UIKRigDefinitionFactory>()));
		if (!TargetRig)
		{
			Report(TEXT("Could not create IK Rig."), true);
			return;
		}
		UIKRigController* Controller = UIKRigController::GetController(TargetRig);
		if (!Controller->SetSkeletalMesh(Mesh))
		{
			Report(TEXT("IK Rig rejected the skeletal mesh."), true);
			return;
		}
		if (Profile.RetargetChains.IsEmpty())
		{
			if (!Controller->ApplyAutoGeneratedRetargetDefinition()) Report(TEXT("Auto retarget chain detection failed. Configure explicit chains in the profile."), true);
		}
		else
		{
			for (const FOmniImportRetargetChain& Chain : Profile.RetargetChains)
			{
				if (Controller->AddRetargetChain(Chain.ChainName, Chain.StartBone, Chain.EndBone, NAME_None).IsNone())
				{
					Report(TEXT("Could not create retarget chain: ") + Chain.ChainName.ToString(), true);
					return;
				}
			}
		}
		Controller->SetRetargetRoot(Profile.RetargetRoot);
		if (Profile.bTryAutoFBIK) ConfigureAutoFBIK(*Controller, *Mesh, Profile.RetargetRoot, Report);
		(void)TargetRig->MarkPackageDirty(); // Companion remains available for explicit saving.
		Report(TEXT("Created IK Rig: ") + TargetRig->GetPathName());
	}
	else if (UIKRigController::GetController(TargetRig)->GetSkeletalMesh() != Mesh)
	{
		Report(TEXT("Existing IK Rig belongs to another mesh; left unchanged: ") + RigPath, true);
		return;
	}
	UIKRigDefinition* SourceRig = Profile.SourceIKRig.LoadSynchronous();
	if (!SourceRig)
	{
		Report(TEXT("Retargeter pending: assign Source IK Rig in the profile."));
		return;
	}
	FString SourceLabel = Profile.SourceRigLabel;
	if (SourceLabel.IsEmpty())
	{
		SourceLabel = SourceRig->GetName();
		SourceLabel.RemoveFromStart(TEXT("IK_"));
	}
	const FString RetargetName = TEXT("IKR_") + SourceLabel + TEXT("To") + ResolvedName;
	if (FPackageName::DoesPackageExist(Folder / RetargetName))
	{
		Report(TEXT("Existing retargeter preserved: ") + RetargetName);
		return;
	}
	UIKRetargeter* Retargeter = Cast<UIKRetargeter>(AssetTools.CreateAsset(RetargetName, Folder, UIKRetargeter::StaticClass(), NewObject<UIKRetargetFactory>()));
	if (!Retargeter)
	{
		Report(TEXT("Could not create IK Retargeter."), true);
		return;
	}
	UIKRetargeterController* Controller = UIKRetargeterController::GetController(Retargeter);
	Controller->SetIKRig(ERetargetSourceOrTarget::Source, SourceRig);
	Controller->SetIKRig(ERetargetSourceOrTarget::Target, TargetRig);
	Controller->AddDefaultOps();
	Controller->AutoMapChains(EAutoMapChainType::Exact, false);
	(void)Retargeter->MarkPackageDirty();
	Report(TEXT("Created retargeter; retarget pose and animation require review: ") + Retargeter->GetPathName());
}
