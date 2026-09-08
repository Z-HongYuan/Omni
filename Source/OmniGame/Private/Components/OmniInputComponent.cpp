// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Components/OmniInputComponent.h"

#include "Components/ExperiencePawnExtensionComponent.h"
#include "CustomInputComponent.h"
#include "CustomInputConfig.h"
#include "Data/ExperiencePawnData.h"
#include "Data/ExperienceSystemTags.h"
#include "EnhancedInputSubsystems.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Gameplay/ExperiencePlayerState.h"
#include "InputActionValue.h"
#include "OmniGame/OmniGameLogChannel.h"
#include "OmniGame/OmniTags.h"
#include "Player/OmniPlayerController.h"
#include "System/CustomAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniInputComponent)

UOmniInputComponent::UOmniInputComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UOmniInputComponent::OnRegister()
{
	Super::OnRegister();

	ensureAlwaysMsgf((GetPawn<APawn>() != nullptr), TEXT("UOmniInputComponent 只能挂到 Pawn 上，当前所有者 [%s] 不是 Pawn。"), *GetNameSafe(GetOwner()));

	// 尽早注册进初始化状态系统（只在游戏世界有效）
	RegisterInitStateFeature();
}

void UOmniInputComponent::BeginPlay()
{
	Super::BeginPlay();

	// 只监听 PawnExtension 的状态变化，由它驱动本组件
	BindOnActorInitStateChanged(UExperiencePawnExtensionComponent::NAME_ActorFeatureName, FGameplayTag(), false);

	// 通知已经生成完毕，然后尝试推进后面的状态
	ensure(TryToChangeInitState(ExperienceSystemTags::TAG_InitState_Spawned));
	CheckDefaultInitialization();
}

void UOmniInputComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 解绑能力输入与输入映射
	if (APawn* Pawn = GetPawn<APawn>())
	{
		if (UCustomInputComponent* CustomIC = Cast<UCustomInputComponent>(Pawn->InputComponent))
		{
			CustomIC->RemoveBinds(AbilityInputBindHandles);

			if (BoundInputConfig)
			{
				if (APlayerController* PC = GetController<APlayerController>())
				{
					if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PC->Player))
					{
						if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
						{
							CustomIC->RemoveInputMappings(BoundInputConfig, Subsystem);
						}
					}
				}
			}
		}
	}
	BoundInputConfig = nullptr;

	// 解绑 GameFeature 注入的额外输入配置（倒序逐个走 RemoveAdditionalInputConfig，保证配对记录同步清理）
	while (!AdditionalInputConfigs.IsEmpty())
	{
		RemoveAdditionalInputConfig(AdditionalInputConfigs.Top().Get());
	}

	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

const FName UOmniInputComponent::NAME_ActorFeatureName(TEXT("InputComponent"));

const FName UOmniInputComponent::NAME_BindInputsReady(TEXT("BindInputsReady"));

bool UOmniInputComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();

	// 无状态 → Spawned：只要 Pawn 存在
	if (!CurrentState.IsValid() && DesiredState == ExperienceSystemTags::TAG_InitState_Spawned)
	{
		if (Pawn) return true;
	}
	// Spawned → DataAvailable
	if (CurrentState == ExperienceSystemTags::TAG_InitState_Spawned && DesiredState == ExperienceSystemTags::TAG_InitState_DataAvailable)
	{
		// 必须有 PlayerState
		if (!GetPlayerState<AExperiencePlayerState>())
		{
			return false;
		}

		// 非模拟端必须已经有控制器，并且控制器与 PlayerState 已配对
		if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
		{
			AController* Controller = GetController<AController>();
			const bool bHasControllerPairedWithPS = (Controller != nullptr) &&
				(Controller->PlayerState != nullptr) &&
				(Controller->PlayerState->GetOwner() == Controller);

			if (!bHasControllerPairedWithPS)
			{
				return false;
			}
		}

		// 本地控制端（非 Bot）必须已有 InputComponent、PlayerController 与 LocalPlayer
		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
		const bool bIsBot = Pawn->IsBotControlled();
		if (bIsLocallyControlled && !bIsBot)
		{
			AOmniPlayerController* OmniPC = GetController<AOmniPlayerController>();
			if (!Pawn->InputComponent || !OmniPC || !OmniPC->GetLocalPlayer())
			{
				return false;
			}
		}

		return true;
	}
	// DataAvailable → DataInitialized：等 PawnExtension 先完成
	if (CurrentState == ExperienceSystemTags::TAG_InitState_DataAvailable && DesiredState == ExperienceSystemTags::TAG_InitState_DataInitialized)
	{
		return GetPlayerState<AExperiencePlayerState>() &&
			Manager->HasFeatureReachedInitState(Pawn, UExperiencePawnExtensionComponent::NAME_ActorFeatureName, ExperienceSystemTags::TAG_InitState_DataInitialized);
	}
	// DataInitialized → GameplayReady
	if (CurrentState == ExperienceSystemTags::TAG_InitState_DataInitialized && DesiredState == ExperienceSystemTags::TAG_InitState_GameplayReady)
	{
		return true;
	}

	return false;
}

void UOmniInputComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	if (CurrentState == ExperienceSystemTags::TAG_InitState_DataAvailable && DesiredState == ExperienceSystemTags::TAG_InitState_DataInitialized)
	{
		APawn* Pawn = GetPawn<APawn>();
		AExperiencePlayerState* PS = GetPlayerState<AExperiencePlayerState>();
		if (!ensure(Pawn && PS))
		{
			return;
		}

		// PlayerState 才是持有 ASC 的一方（数据跨死亡持久、挂在 PS 上）
		// 因此在所有端都要把 ASC 接到 PawnExtension 上
		if (UExperiencePawnExtensionComponent* PawnExtComp = UExperiencePawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			PawnExtComp->InitializeAbilitySystem(PS->GetCustomAbilitySystemComponent(), PS);
		}

		// 只有本地控制端（非 Bot）才绑输入，Bot 走到这里只完成 ASC 初始化
		if (Pawn->IsLocallyControlled() && !Pawn->IsBotControlled() && Pawn->InputComponent)
		{
			InitializePlayerInput(Pawn->InputComponent);
		}
	}
}

void UOmniInputComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// 只监听 PawnExtension（BeginPlay 里的 Bind 已限定），它到 DataInitialized 时推自己一把
	check(Params.FeatureName == UExperiencePawnExtensionComponent::NAME_ActorFeatureName);

	if (Params.FeatureState == ExperienceSystemTags::TAG_InitState_DataInitialized)
	{
		CheckDefaultInitialization();
	}
}

void UOmniInputComponent::CheckDefaultInitialization()
{
	// 先让依赖本组件的其他特性尝试推进，再按状态链推自己
	// CheckDefaultInitializationForImplementers();

	ContinueInitStateChain(ExperienceSystemTags::ComponentStateChain);
}

void UOmniInputComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	APawn* Pawn = GetPawn<APawn>();
	APlayerController* PC = GetController<APlayerController>();
	if (!Pawn || !PC)
	{
		return;
	}

	// 从 PawnExtension 拿 PawnData，再拿输入配置
	const UExperiencePawnData* PawnData = nullptr;
	if (UExperiencePawnExtensionComponent* PawnExtComp = UExperiencePawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		PawnData = PawnExtComp->GetPawnData<UExperiencePawnData>();
	}
	if (!PawnData || !PawnData->InputConfig)
	{
		UE_LOG(LogOmniGame, Warning, TEXT("[%s] PawnData 未配置 InputConfig，跳过输入绑定。"), *GetNameSafe(Pawn));
		return;
	}

	// 必须是自定义的输入组件，否则无法绑定能力输入
	UCustomInputComponent* CustomIC = Cast<UCustomInputComponent>(PlayerInputComponent);
	if (!CustomIC)
	{
		UE_LOG(LogOmniGame, Error, TEXT("[%s] PlayerController 的 InputComponentClass 需要设置为 UCustomInputComponent，当前为 [%s]，无法绑定能力输入。"),
		       *GetNameSafe(PC), *GetNameSafe(PlayerInputComponent));
		return;
	}

	if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PC->Player))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			BoundInputConfig = PawnData->InputConfig;

			// 注册输入映射
			CustomIC->AddInputMappings(BoundInputConfig, Subsystem);

			// 能力输入：按下/抬起转发给 ASC，由 ASC 按 InputTag 触发对应能力
			CustomIC->BindAbilityActions(BoundInputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, AbilityInputBindHandles);

			// 基础原生动作：直接驱动 Pawn，不走 ASC
			// InputConfig 里没配对应 Tag 的 InputAction 时只是跳过，不会报错
			CustomIC->BindNativeAction(BoundInputConfig, OmniTags::TAG_InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, false);
			CustomIC->BindNativeAction(BoundInputConfig, OmniTags::TAG_InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_Look_Mouse, false);
		}
	}

	// 标记输入绑定完成；GameFeatureAction_AddInputBinding 依赖此标记判断能否立即挂输入
	if (ensure(!bReadyToBindInputs))
	{
		bReadyToBindInputs = true;
	}

	// 广播输入绑定完成，给 GameFeature Action（AddInputBinding / AddInputContextMapping）挂输入的时机
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(PC, NAME_BindInputsReady);
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(Pawn, NAME_BindInputsReady);
}

void UOmniInputComponent::Input_AbilityInputTagPressed(const FGameplayTag InputTag)
{
	if (const UExperiencePawnExtensionComponent* PawnExtComp = UExperiencePawnExtensionComponent::FindPawnExtensionComponent(GetPawn<APawn>()))
	{
		if (UCustomAbilitySystemComponent* ASC = PawnExtComp->GetCustomAbilitySystemComponent())
		{
			ASC->AbilityInputTagPressed(InputTag);
		}
	}
}

void UOmniInputComponent::Input_AbilityInputTagReleased(const FGameplayTag InputTag)
{
	if (const UExperiencePawnExtensionComponent* PawnExtComp = UExperiencePawnExtensionComponent::FindPawnExtensionComponent(GetPawn<APawn>()))
	{
		if (UCustomAbilitySystemComponent* ASC = PawnExtComp->GetCustomAbilitySystemComponent())
		{
			ASC->AbilityInputTagReleased(InputTag);
		}
	}
}

void UOmniInputComponent::Input_Move(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	AController* Controller = Pawn ? Pawn->GetController() : nullptr;
	if (!Controller) return;

	// 用控制器的偏航角确定移动朝向（WASD 相对于视角方向）
	const FVector2D Value = InputActionValue.Get<FVector2D>();
	const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

	if (Value.X != 0.0f)
	{
		const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
		Pawn->AddMovementInput(MovementDirection, Value.X);
	}

	if (Value.Y != 0.0f)
	{
		const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
		Pawn->AddMovementInput(MovementDirection, Value.Y);
	}
}

void UOmniInputComponent::Input_Look_Mouse(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	const FVector2D Value = InputActionValue.Get<FVector2D>();
	if (Value.X != 0.0f)
	{
		Pawn->AddControllerYawInput(Value.X);
	}
	if (Value.Y != 0.0f)
	{
		Pawn->AddControllerPitchInput(Value.Y);
	}
}

void UOmniInputComponent::AddAdditionalInputConfig(const UCustomInputConfig* InputConfig)
{
	// 同一份配置只挂一次：激活线有 ExtensionAdded / BindInputsReady 两个触发事件，重复进入会重复绑定
	if (!InputConfig || AdditionalInputConfigs.Contains(InputConfig))
	{
		return;
	}

	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	// 能力输入挂在 PlayerController 的输入组件上（Pawn->InputComponent 持有引用），必须是自定义输入组件
	if (UCustomInputComponent* CustomIC = Cast<UCustomInputComponent>(Pawn->InputComponent))
	{
		TArray<uint32> BindHandles;
		CustomIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);

		AdditionalInputConfigs.Add(InputConfig);
		AdditionalAbilityBindHandles.Add(BindHandles);
	}
	else
	{
		UE_LOG(LogOmniGame, Error, TEXT("[%s] PlayerController 的 InputComponentClass 需要设置为 UCustomInputComponent，无法挂载额外输入配置 [%s]。"),
		       *GetNameSafe(GetController()), *GetNameSafe(InputConfig));
	}
}

void UOmniInputComponent::RemoveAdditionalInputConfig(const UCustomInputConfig* InputConfig)
{
	if (!InputConfig)
	{
		return;
	}

	// Lyra 此函数是 @TODO 空实现；这里补全为真实解绑，保证 GameFeature 反激活后输入不残留
	int32 ConfigIndex = INDEX_NONE;
	for (int32 Index = 0; Index < AdditionalInputConfigs.Num(); ++Index)
	{
		if (AdditionalInputConfigs[Index].Get() == InputConfig)
		{
			ConfigIndex = Index;
			break;
		}
	}
	if (ConfigIndex == INDEX_NONE)
	{
		return;
	}

	if (APawn* Pawn = GetPawn<APawn>())
	{
		if (UCustomInputComponent* CustomIC = Cast<UCustomInputComponent>(Pawn->InputComponent))
		{
			CustomIC->RemoveBinds(AdditionalAbilityBindHandles[ConfigIndex]);
		}
	}

	// 两个数组按下标配对，必须同步删除
	AdditionalInputConfigs.RemoveAt(ConfigIndex);
	AdditionalAbilityBindHandles.RemoveAt(ConfigIndex);
}
