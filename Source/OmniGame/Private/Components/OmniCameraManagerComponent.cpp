// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Components/OmniCameraManagerComponent.h"

#include "Components/ExperiencePawnExtensionComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Data/ExperiencePawnData.h"
#include "Data/ExperienceSystemTags.h"
#include "GameFramework/GameplayCameraComponent.h"
#include "OmniGame/OmniGameLogChannel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniCameraManagerComponent)

const FName UOmniCameraManagerComponent::NAME_ActorFeatureName(TEXT("CameraManager"));
const FName UOmniCameraManagerComponent::NAME_CameraReady(TEXT("CameraReady"));

UOmniCameraManagerComponent::UOmniCameraManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
}

void UOmniCameraManagerComponent::OnRegister()
{
	Super::OnRegister();

	ensureAlwaysMsgf((GetPawn<APawn>() != nullptr), TEXT("UOmniCameraManagerComponent 只能挂到 Pawn 上，当前所有者 [%s] 不是 Pawn。"), *GetNameSafe(GetOwner()));

	RegisterInitStateFeature();
}

void UOmniCameraManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	BindOnActorInitStateChanged(UExperiencePawnExtensionComponent::NAME_ActorFeatureName, FGameplayTag(), false);

	ensure(TryToChangeInitState(ExperienceSystemTags::TAG_InitState_Spawned));
	CheckDefaultInitialization();
}

void UOmniCameraManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

bool UOmniCameraManagerComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
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

void UOmniCameraManagerComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	// 设置相机资产：本地控制端才会用到相机
	if (CurrentState == ExperienceSystemTags::TAG_InitState_DataAvailable && DesiredState == ExperienceSystemTags::TAG_InitState_DataInitialized)
	{
		if (APawn* Pawn = GetPawn<APawn>(); Pawn && Pawn->IsLocallyControlled())
		{
			if (UGameplayCameraComponent* CameraComponent = FindGameplayCameraComponent())
			{
				// GameFeatureAction_AddComponents 注册的组件不会自动挂到 Pawn 上（不挂会在原点，相机画面错位），这里补挂到根组件；
				// 已有挂接（如 Pawn 蓝图自带）则不干预。
				// 踩坑：已注册组件不能用 SetupAttachment（引擎里 ensure 会失败），运行期挂接必须用 AttachToComponent。
				if (!CameraComponent->GetAttachParent() && Pawn->GetRootComponent())
				{
					CameraComponent->AttachToComponent(Pawn->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
				}

				if (UCameraAsset* CameraAsset = GetCameraAssetFromPawnData())
				{
					// SetCameraAsset 是内联函数，内部调用已导出的 RebuildParameters()，跨模块调用安全
					CameraComponent->CameraReference.SetCameraAsset(CameraAsset);
				}
				else
				{
					UE_LOG(LogOmniGame, Warning, TEXT("[%s] PawnData 未配置 CameraAsset，相机组件将使用其上已配置的资产。"), *GetNameSafe(Pawn));
				}
			}
			else
			{
				UE_LOG(LogOmniGame, Warning, TEXT("[%s] Pawn 附加的组件中没有 UGameplayCameraComponent，相机管理不生效，请检查 GameFeatureAction_AddComponents 的组件配置。"), *GetNameSafe(Pawn));
			}
		}
	}

	// 激活相机：等一切就绪
	if (CurrentState == ExperienceSystemTags::TAG_InitState_DataInitialized && DesiredState == ExperienceSystemTags::TAG_InitState_GameplayReady)
	{
		if (APawn* Pawn = GetPawn<APawn>(); Pawn && Pawn->IsLocallyControlled())
		{
			// 上一步没找到相机时这里会再查一次（兼容体验资产较晚附加相机的场景）
			if (UGameplayCameraComponent* CameraComponent = FindGameplayCameraComponent())
			{
				if (APlayerController* PC = GetController<APlayerController>())
				{
					// bSetAsViewTarget=true 会把本组件设为视角目标
					CameraComponent->ActivateCameraForPlayerController(PC);

					OnCameraReady(GetCameraAssetFromPawnData());

					// 广播相机就绪，给 GameFeature Action 挂相机相关内容（如持久相机 Rig、相机层）的时机
					// 发给 PC 和 Pawn 两个接收者
					UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(PC, NAME_CameraReady);
					UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(Pawn, NAME_CameraReady);
				}
			}
		}
	}
}

void UOmniCameraManagerComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	check(Params.FeatureName == UExperiencePawnExtensionComponent::NAME_ActorFeatureName);

	if (Params.FeatureState == ExperienceSystemTags::TAG_InitState_DataInitialized)
	{
		CheckDefaultInitialization();
	}
}

void UOmniCameraManagerComponent::CheckDefaultInitialization()
{
	// 感觉没必要推进
	// CheckDefaultInitializationForImplementers();

	ContinueInitStateChain(ExperienceSystemTags::ComponentStateChain);
}

UGameplayCameraComponent* UOmniCameraManagerComponent::FindGameplayCameraComponent()
{
	// 已缓存直接返回，避免每次状态迁移都重复遍历组件列表
	if (GameplayCameraComponent)
	{
		return GameplayCameraComponent;
	}

	// 按类文档：相机不是本组件创建的，必须已在 Pawn 附加的组件里（Pawn 自带或 GameFeatureAction 平级添加），用 Find 查找
	if (APawn* Pawn = GetPawn<APawn>())
	{
		GameplayCameraComponent = Pawn->FindComponentByClass<UGameplayCameraComponent>();
	}

	return GameplayCameraComponent;
}

UCameraAsset* UOmniCameraManagerComponent::GetCameraAssetFromPawnData() const
{
	const UExperiencePawnExtensionComponent* PawnExtComp = UExperiencePawnExtensionComponent::FindPawnExtensionComponent(GetPawn<APawn>());
	const UExperiencePawnData* PawnData = PawnExtComp ? PawnExtComp->GetPawnData<UExperiencePawnData>() : nullptr;

	return PawnData ? PawnData->CameraAsset : nullptr;
}

void UOmniCameraManagerComponent::OnCameraReady(UCameraAsset* CameraAsset)
{
	// C++ 派生类重载本函数；这里转发给蓝图事件，否则蓝图派生类重载 OnCameraReady 永远收不到回调
	K2_OnCameraReady(CameraAsset);
}
