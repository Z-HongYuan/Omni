// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Camera/OmniCameraManagerComponent.h"

#include "CineCameraComponent.h"
#include "Components/ExpPawnExtensionComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Data/ExpPawnData.h"
#include "Data/ExpSystemTags.h"
#include "GameFramework/GameplayCameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "OmniGame/OmniGameLogChannel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniCameraManagerComponent)

UOmniCameraManagerComponent::UOmniCameraManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UOmniCameraManagerComponent::OnRegister()
{
	Super::OnRegister();

	if (ensure(GetPawn<APawn>())) RegisterInitStateFeature();
}

void UOmniCameraManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	BindOnActorInitStateChanged(UExpPawnExtensionComponent::NAME_ActorFeatureName, FGameplayTag(), false);
	ensure(TryToChangeInitState(ExpSystemTags::TAG_InitState_Spawned));
	CheckDefaultInitialization();
}

void UOmniCameraManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();

	ReleaseCamera();

	Super::EndPlay(EndPlayReason);
}

const FName UOmniCameraManagerComponent::NAME_ActorFeatureName("CameraManager");

bool UOmniCameraManagerComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();
	if (!CurrentState.IsValid() && DesiredState == ExpSystemTags::TAG_InitState_Spawned)
	{
		return Pawn != nullptr;
	}

	const UExpPawnExtensionComponent* Extension = UExpPawnExtensionComponent::FindPawnExtensionComponent(Pawn);

	if (CurrentState == ExpSystemTags::TAG_InitState_Spawned && DesiredState == ExpSystemTags::TAG_InitState_DataAvailable)
	{
		// 只等待数据，不等待 ASC 或本地相机激活，避免阻塞服务器及其他功能。
		return Extension && Extension->GetPawnData<UExpPawnData>();
	}
	if (CurrentState == ExpSystemTags::TAG_InitState_DataAvailable && DesiredState == ExpSystemTags::TAG_InitState_DataInitialized)
	{
		return Manager->HasFeatureReachedInitState(Pawn, UExpPawnExtensionComponent::NAME_ActorFeatureName, ExpSystemTags::TAG_InitState_DataInitialized);
	}
	if (CurrentState == ExpSystemTags::TAG_InitState_DataInitialized && DesiredState == ExpSystemTags::TAG_InitState_GameplayReady)
	{
		return true;
	}

	return false;
}

void UOmniCameraManagerComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	if (CurrentState == ExpSystemTags::TAG_InitState_DataAvailable && DesiredState == ExpSystemTags::TAG_InitState_DataInitialized)
	{
		UpdateCamera();
	}
}

void UOmniCameraManagerComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName == UExpPawnExtensionComponent::NAME_ActorFeatureName
		&& Params.FeatureState == ExpSystemTags::TAG_InitState_DataInitialized)
	{
		CheckDefaultInitialization();
	}
}

void UOmniCameraManagerComponent::CheckDefaultInitialization()
{
	ContinueInitStateChain(ExpSystemTags::ComponentStateChain);
}

void UOmniCameraManagerComponent::UpdateCamera()
{
	APawn* Pawn = GetPawn<APawn>();
	APlayerController* Controller = GetController<APlayerController>();
	const UExpPawnExtensionComponent* Extension = UExpPawnExtensionComponent::FindPawnExtensionComponent(Pawn);
	const UExpPawnData* Data = Extension ? Extension->GetPawnData<UExpPawnData>() : nullptr;

	// 确保在需要的地方才使用摄像机,不然就释放
	if (!Controller || !Controller->IsLocalController() || !Controller->GetLocalPlayer())
	{
		ReleaseCamera();
		LastCameraIssue = NAME_None;
		return;
	}

	// 相机由蓝图/GF 提供；只接受唯一已注册实例，避免任意选中多相机中的一个。
	TInlineComponentArray<UGameplayCameraComponent*> Cameras(Pawn);
	Cameras.RemoveAll([](const UGameplayCameraComponent* Camera) { return !IsValid(Camera) || !Camera->IsRegistered(); });

	//需要有且仅有一个
	if (Cameras.Num() != 1)
	{
		ReleaseCamera();
		ReportCameraIssue(Cameras.IsEmpty() ? FName("MissingCamera") : FName("MultipleCameras"),
		                  Cameras.IsEmpty()
			                  ? TEXT("No registered GameplayCamera found; skipping camera control.")
			                  : TEXT("Multiple GameplayCameras found; camera control requires one registered component."));
		return;
	}

	// 需要数据中的摄像机配置有效
	if (!Data || !Data->CameraAsset)
	{
		ReleaseCamera();
		ReportCameraIssue(TEXT("MissingCameraAsset"), TEXT("PawnData has no CameraAsset; skipping camera control."));
		return;
	}

	UGameplayCameraComponent* Camera = Cameras[0];
	LastCameraIssue = NAME_None;

	// 摄像机、控制器和配置未变化时，无需重复接管。
	if (CameraComponent == Camera && CameraController == Controller && Camera->CameraReference.GetCameraAsset() == Data->CameraAsset)
	{
		return;
	}

	ReleaseCamera();
	// 已有相机可能自动激活过；先停止旧求值上下文，再应用配置和本地玩家，确保新配置实际生效。
	Camera->Deactivate();
	Camera->CameraReference.SetCameraAsset(Data->CameraAsset);

	// 使用当前玩家，避免分屏时落到 Player0；视角目标仍由 PC/玩法管理。
	Camera->ActivateCameraForPlayerController(Controller);

	// UE 5.8 的指定玩家激活接口不会重新启用已有输出相机，否则视点会退回 Pawn 眼睛位置。(修复客户端中的摄像机问题)
	if (UCineCameraComponent* OutputCamera = Camera->GetOutputCameraComponent()) OutputCamera->Activate();

	CameraComponent = Camera;
	CameraController = Controller;
}

void UOmniCameraManagerComponent::ReleaseCamera()
{
	if (UGameplayCameraComponent* Camera = CameraComponent.Get())
	{
		// 外部若已把相机转交其他玩家，旧管理器不能停用新的控制关系。
		const auto Context = Camera->GetEvaluationContext();
		if (Context && Context->GetPlayerController() == CameraController.Get())
		{
			Camera->Deactivate();
		}
	}

	CameraComponent.Reset();
	CameraController.Reset();
}

void UOmniCameraManagerComponent::ReportCameraIssue(FName Issue, const TCHAR* Message)
{
	// 同一缺失状态只记录一次；正常接管后再次缺失时允许重新记录。
	if (LastCameraIssue != Issue)
	{
		UE_LOG(LogOmniGame, Log, TEXT("CameraManager: %s Pawn=%s"), Message, *GetPathNameSafe(GetOwner()));
		LastCameraIssue = Issue;
	}
}
