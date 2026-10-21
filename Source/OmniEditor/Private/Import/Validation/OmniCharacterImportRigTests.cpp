// Copyright © 2026 张鸿源. All Rights Reserved.
#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Import/Assets/OmniImportRigAssets.h"
#include "Import/OmniCharacterImportProfile.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/SkeletalMesh.h"
#include "Misc/Guid.h"
#include "Misc/PackageName.h"
#include "Rig/IKRigDefinition.h"
#include "Rig/Solvers/IKRigFullBodyIK.h"
#include "RigEditor/IKRigController.h"
#include "UObject/Package.h"

namespace
{
struct FScopedImportRigFixture
{
	FString PackageName;
	FString ObjectPath;

	~FScopedImportRigFixture()
	{
		// Only detach this test's unique, unsaved asset. Never delete packages from disk.
		UObject* Asset = FindObject<UObject>(nullptr, *ObjectPath);
		if (!Asset || Asset->GetOutermost()->GetName() != PackageName) return;
		UPackage* Package = Asset->GetOutermost();
		FAssetRegistryModule::AssetDeleted(Asset);
		Asset->ClearFlags(RF_Public | RF_Standalone);
		Asset->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors | REN_NonTransactional);
		Package->SetDirtyFlag(false);
	}
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOmniImportFullBodyIKTest, "Omni.Editor.CharacterImport.FullBodyIKCreation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FOmniImportFullBodyIKTest::RunTest(const FString& Parameters)
{
	UIKRigDefinition* SourceRig = LoadObject<UIKRigDefinition>(nullptr,
		TEXT("/Game/Art/BasicHuman/Mannequin/Rig/IK_Mannequin.IK_Mannequin"));
	if (!TestNotNull(TEXT("Read-only official IK Rig fixture"), SourceRig)) return false;
	UIKRigController* SourceController = UIKRigController::GetController(SourceRig);
	USkeletalMesh* Mesh = SourceController->GetSkeletalMesh();
	if (!TestNotNull(TEXT("Official Rig's preview skeletal mesh"), Mesh)) return false;
	const int32 SourceSolverCount = SourceController->GetNumSolvers();
	const int32 SourceGoalCount = SourceController->GetAllGoals().Num();

	const FString Name = TEXT("RigFixture_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
	const FString Root = TEXT("/Game/ImportTests/") + Name;
	const FString RigName = TEXT("IK_") + Name;
	const FString PackageName = Root / TEXT("Rig") / RigName;
	const FString ObjectPath = PackageName + TEXT(".") + RigName;
	if (!TestFalse(TEXT("Fixture destination does not exist on disk"), FPackageName::DoesPackageExist(PackageName))) return false;
	if (!TestNull(TEXT("Fixture destination is not already loaded"), FindObject<UObject>(nullptr, *ObjectPath))) return false;
	FScopedImportRigFixture Fixture{PackageName, ObjectPath};

	UOmniCharacterImportProfile* Profile = NewObject<UOmniCharacterImportProfile>(GetTransientPackage());
	Profile->bCreateIKRig = true;
	Profile->bTryAutoFBIK = true;
	Profile->RigFolder = TEXT("Rig");
	Profile->RetargetRoot = TEXT("pelvis");
	Profile->SourceIKRig.Reset(); // Exercise target rig creation without creating a retargeter.
	TArray<FString> Errors;
	const Omni::CharacterImport::FImportReporter Reporter{[&Errors](const FString& Message, bool bError)
	{
		if (bError) Errors.Add(Message);
	}};
	Omni::CharacterImport::CreateRigAssets(Mesh, *Profile, Root, Name, Reporter);
	TestTrue(*FString::Printf(TEXT("Rig creation reports no errors: %s"), *FString::Join(Errors, TEXT("; "))), Errors.IsEmpty());
	UIKRigDefinition* Rig = FindObject<UIKRigDefinition>(nullptr, *Fixture.ObjectPath);
	if (!TestNotNull(TEXT("New rig was created in memory"), Rig)) return false;
	TestTrue(TEXT("Target fixture is a separate asset"), Rig != SourceRig);
	UIKRigController* Controller = UIKRigController::GetController(Rig);
	TestEqual(TEXT("New rig references the official mesh"), Controller->GetSkeletalMesh(), Mesh);
	if (!TestEqual(TEXT("Exactly one solver is created"), Controller->GetNumSolvers(), 1)) return false;
	FInstancedStruct* SolverStruct = Controller->GetSolverStructAtIndex(0);
	FIKRigFullBodyIKSolver* Solver = SolverStruct ? SolverStruct->GetMutablePtr<FIKRigFullBodyIKSolver>() : nullptr;
	if (!TestNotNull(TEXT("Solver is the UE Full Body IK implementation"), Solver)) return false;
	TestTrue(TEXT("Full Body IK solver is enabled"), Solver->IsEnabled());
	TestEqual(TEXT("Solver uses the profile root"), Controller->GetStartBone(0), Profile->RetargetRoot);
	TestEqual(TEXT("Four effector goals are created"), Controller->GetAllGoals().Num(), 4);
	TMap<FName, UIKRigEffectorGoal*> OriginalGoals;
	for (const FName ChainName : {FName(TEXT("LeftArm")), FName(TEXT("RightArm")), FName(TEXT("LeftLeg")), FName(TEXT("RightLeg"))})
	{
		const FBoneChain* Chain = Controller->GetRetargetChainByName(ChainName);
		if (!TestNotNull(*FString::Printf(TEXT("Retarget chain %s exists"), *ChainName.ToString()), Chain)) continue;
		UIKRigEffectorGoal* Goal = Controller->GetGoal(Chain->IKGoalName);
		if (!TestNotNull(*FString::Printf(TEXT("Chain %s has an effector"), *ChainName.ToString()), Goal)) continue;
		TestEqual(TEXT("Effector belongs to its chain's end bone"), Goal->BoneName, Chain->EndBone.BoneName);
		TestTrue(TEXT("Effector is connected to Full Body IK"), Controller->IsGoalConnectedToSolver(Goal->GoalName, 0));
		OriginalGoals.Add(Goal->GoalName, Goal);
	}

	Solver->Settings.Iterations = 37; // User tuning on the disposable fixture must survive the second call.
	Omni::CharacterImport::CreateRigAssets(Mesh, *Profile, Root, Name, Reporter);
	TestEqual(TEXT("Second call reuses the same rig"), FindObject<UIKRigDefinition>(nullptr, *Fixture.ObjectPath), Rig);
	TestEqual(TEXT("Second call preserves the solver count"), Controller->GetNumSolvers(), 1);
	TestEqual(TEXT("Second call preserves the same solver"), Controller->GetSolverStructAtIndex(0), SolverStruct);
	FInstancedStruct* FinalSolverStruct = Controller->GetSolverStructAtIndex(0);
	const FIKRigFullBodyIKSolver* FinalSolver = FinalSolverStruct ? FinalSolverStruct->GetPtr<FIKRigFullBodyIKSolver>() : nullptr;
	if (TestNotNull(TEXT("Second call retains Full Body IK"), FinalSolver))
	{
		TestEqual(TEXT("Second call preserves user solver tuning"), FinalSolver->Settings.Iterations, 37);
	}
	TestEqual(TEXT("Second call preserves the goal count"), Controller->GetAllGoals().Num(), 4);
	for (const auto& Pair : OriginalGoals) TestEqual(TEXT("Second call preserves each goal"), Controller->GetGoal(Pair.Key), Pair.Value);
	TestTrue(TEXT("Repeated rig creation reports no errors"), Errors.IsEmpty());
	TestEqual(TEXT("Official rig solver count remains unchanged"), SourceController->GetNumSolvers(), SourceSolverCount);
	TestEqual(TEXT("Official rig goal count remains unchanged"), SourceController->GetAllGoals().Num(), SourceGoalCount);
	TestFalse(TEXT("AssetTools creation did not save the fixture"), FPackageName::DoesPackageExist(PackageName));
	return true;
}

#endif
