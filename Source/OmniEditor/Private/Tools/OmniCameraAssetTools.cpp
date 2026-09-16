// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Tools/OmniCameraAssetTools.h"

#include "Core/CameraAsset.h"
#include "Core/CameraRigAsset.h"
#include "Directors/SingleCameraDirector.h"
#include "Nodes/Common/ArrayCameraNode.h"
#include "Nodes/Common/OffsetCameraNode.h"
#include "Nodes/Common/SetRotationCameraNode.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniCameraAssetTools)

bool UOmniCameraAssetTools::ConfigureFixedCamera(UCameraAsset* Camera, UCameraRigAsset* Rig, FVector WorldOffset, FRotator WorldRotation)
{
	if (!Camera || !Rig || WorldOffset.ContainsNaN() || WorldRotation.ContainsNaN()) return false;
	Camera->Modify();
	Rig->Modify();
	UArrayCameraNode* Sequence = NewObject<UArrayCameraNode>(Rig, NAME_None, RF_Transactional);
	UOffsetCameraNode* Offset = NewObject<UOffsetCameraNode>(Rig, NAME_None, RF_Transactional);
	Offset->TranslationOffset.Value = WorldOffset;
	Offset->OffsetSpace = ECameraNodeSpace::World;
	USetRotationCameraNode* Rotation = NewObject<USetRotationCameraNode>(Rig, NAME_None, RF_Transactional);
	Rotation->Rotation.Value = WorldRotation;
	Rotation->OffsetSpace = ECameraNodeSpace::World;
	Sequence->Children = {Offset, Rotation};
	Rig->RootNode = Sequence;
	USingleCameraDirector* Director = NewObject<USingleCameraDirector>(Camera, NAME_None, RF_Transactional);
	Director->CameraRig = Rig;
	Camera->SetCameraDirector(Director);
	Rig->BuildCameraRig();
	Camera->BuildCamera();
	Camera->MarkPackageDirty();
	Rig->MarkPackageDirty();
	return Camera->GetBuildStatus() != ECameraBuildStatus::WithErrors;
}
