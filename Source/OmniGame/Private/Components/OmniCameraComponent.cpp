// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Components/OmniCameraComponent.h"

#include "Components/ExperiencePawnExtensionComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Data/ExperiencePawnData.h"
#include "Data/ExperienceSystemTags.h"
#include "GameFramework/GameplayCameraComponent.h"
#include "OmniGame/OmniGameLogChannel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniCameraComponent)

const FName UOmniCameraComponent::NAME_ActorFeatureName(TEXT("CameraManager"));
const FName UOmniCameraComponent::NAME_CameraReady(TEXT("CameraReady"));

UOmniCameraComponent::UOmniCameraComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);

	// 引擎的相机组件负责真正的相机运行
	GameplayCameraComponent = ObjectInitializer.CreateDefaultSubobject<UGameplayCameraComponent>(this, TEXT("GameplayCameraComponent"));
}

void UOmniCameraComponent::OnRegister()
{
	Super::OnRegister();

	ensureAlwaysMsgf((GetPawn<APawn>() != nullptr), TEXT("UOmniCameraComponent 只能挂到 Pawn 上，当前所有者 [%s] 不是 Pawn。"), *GetNameSafe(GetOwner()));

	RegisterInitStateFeature();
}

void UOmniCameraComponent::BeginPlay()
{
	Super::BeginPlay();

	BindOnActorInitStateChanged(UExperiencePawnExtensionComponent::NAME_ActorFeatureName, FGameplayTag(), false);

	ensure(TryToChangeInitState(ExperienceSystemTags::TAG_InitState_Spawned));
	CheckDefaultInitialization();
}

void UOmniCameraComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

bool UOmniCameraComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();

	// 无状态 → Spawned：只要 Pawn 存在
	if (!CurrentState.IsValid() && DesiredState == ExperienceSystemTags::TAG_InitState_Spawned)
	{
		if (Pawn) return true;
	}
	// Spawned → DataAvailable：PawnExtension 的 PawnData 就绪即可（相机不需要控制器）
	if (CurrentState == ExperienceSystemTags::TAG_InitState_Spawned && DesiredState == ExperienceSystemTags::TAG_InitState_DataAvailable)
	{
		const UExperiencePawnExtensionComponent* PawnExtComp = UExperiencePawnExtensionComponent::FindPawnExtensionComponent(Pawn);
		return (PawnExtComp != nullptr) && (PawnExtComp->GetPawnData<UExperiencePawnData>() != nullptr);
	}
	// DataAvailable → DataInitialized：等 PawnExtension 先完成
	if (CurrentState == ExperienceSystemTags::TAG_InitState_DataAvailable && DesiredState == ExperienceSystemTags::TAG_InitState_DataInitialized)
	{
		return Manager->HasFeatureReachedInitState(Pawn, UExperiencePawnExtensionComponent::NAME_ActorFeatureName, ExperienceSystemTags::TAG_InitState_DataInitialized);
	}
	// DataInitialized → GameplayReady
	if (CurrentState == ExperienceSystemTags::TAG_InitState_DataInitialized && DesiredState == ExperienceSystemTags::TAG_InitState_GameplayReady)
	{
		return true;
	}

	return false;
}

void UOmniCameraComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	// 设置相机资产：本地控制端才会用到相机
	if (CurrentState == ExperienceSystemTags::TAG_InitState_DataAvailable && DesiredState == ExperienceSystemTags::TAG_InitState_DataInitialized)
	{
		APawn* Pawn = GetPawn<APawn>();
		if (Pawn && Pawn->IsLocallyControlled() && GameplayCameraComponent)
		{
			if (UCameraAsset* CameraAsset = GetCameraAssetFromPawnData())
			{
				// SetCameraAsset 是内联函数，内部调用已导出的 RebuildParameters()，跨模块调用安全
				GameplayCameraComponent->CameraReference.SetCameraAsset(CameraAsset);
			}
			else
			{
				UE_LOG(LogOmniGame, Warning, TEXT("[%s] PawnData 未配置 CameraAsset，相机组件将使用 GameplayCameraComponent 上已配置的资产。"), *GetNameSafe(Pawn));
			}
		}
	}

	// 激活相机：等一切就绪
	if (CurrentState == ExperienceSystemTags::TAG_InitState_DataInitialized && DesiredState == ExperienceSystemTags::TAG_InitState_GameplayReady)
	{
		APawn* Pawn = GetPawn<APawn>();
		if (Pawn && Pawn->IsLocallyControlled() && GameplayCameraComponent)
		{
			if (APlayerController* PC = GetController<APlayerController>())
			{
				// bSetAsViewTarget=true 会把本组件设为视角目标
				GameplayCameraComponent->ActivateCameraForPlayerController(PC);

				OnCameraReady(GetCameraAssetFromPawnData());

				// 广播相机就绪，给 GameFeature Action 挂相机相关内容（如持久相机 Rig、相机层）的时机
				// 对应 Lyra 的 NAME_BindInputsNow 模式，发给 PC 和 Pawn 两个接收者
				UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(PC, NAME_CameraReady);
				UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(Pawn, NAME_CameraReady);
			}
		}
	}
}

void UOmniCameraComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	check(Params.FeatureName == UExperiencePawnExtensionComponent::NAME_ActorFeatureName);

	if (Params.FeatureState == ExperienceSystemTags::TAG_InitState_DataInitialized)
	{
		CheckDefaultInitialization();
	}
}

void UOmniCameraComponent::CheckDefaultInitialization()
{
	// 感觉没必要推进
	// CheckDefaultInitializationForImplementers();

	ContinueInitStateChain(ExperienceSystemTags::ComponentStateChain);
}

UCameraAsset* UOmniCameraComponent::GetCameraAssetFromPawnData() const
{
	const UExperiencePawnExtensionComponent* PawnExtComp = UExperiencePawnExtensionComponent::FindPawnExtensionComponent(GetPawn<APawn>());
	const UExperiencePawnData* PawnData = PawnExtComp ? PawnExtComp->GetPawnData<UExperiencePawnData>() : nullptr;

	return PawnData ? PawnData->CameraAsset : nullptr;
}

void UOmniCameraComponent::OnCameraReady(UCameraAsset* CameraAsset)
{
	// 空实现，供派生类重载
}
