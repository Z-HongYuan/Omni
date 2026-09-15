// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Input/OmniInputManagerComponent.h"

#include "Components/ExpPawnExtensionComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Data/ExpPawnData.h"
#include "Data/ExpSystemTags.h"
#include "GameFramework/PlayerController.h"
#include "Input/ExtInputComponent.h"
#include "Input/ExtInputConfig.h"
#include "InputActionValue.h"
#include "OmniGame/OmniTags.h"
#include "System/ExtAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniInputManagerComponent)

UOmniInputManagerComponent::UOmniInputManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UOmniInputManagerComponent::OnRegister()
{
	Super::OnRegister();

	if (ensure(GetPawn<APawn>())) RegisterInitStateFeature();
}

void UOmniInputManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	BindOnActorInitStateChanged(UExpPawnExtensionComponent::NAME_ActorFeatureName, FGameplayTag(), false);
	ensure(TryToChangeInitState(ExpSystemTags::TAG_InitState_Spawned));
	CheckDefaultInitialization();
}

void UOmniInputManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();

	// 仅移除本组件创建的动作绑定
	ReleaseInputBind();

	Super::EndPlay(EndPlayReason);
}

const FName UOmniInputManagerComponent::NAME_ActorFeatureName("InputManager");
const FName UOmniInputManagerComponent::NAME_BindInputsNow("BindInputsNow");

bool UOmniInputManagerComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();
	if (!CurrentState.IsValid() && DesiredState == ExpSystemTags::TAG_InitState_Spawned)
	{
		return Pawn != nullptr;
	}

	const UExpPawnExtensionComponent* Extension = UExpPawnExtensionComponent::FindPawnExtensionComponent(Pawn);
	const UExpPawnData* Data = Extension ? Extension->GetPawnData<UExpPawnData>() : nullptr;

	if (CurrentState == ExpSystemTags::TAG_InitState_Spawned && DesiredState == ExpSystemTags::TAG_InitState_DataAvailable)
	{
		if (!Data) return false;

		// 尚未确定控制关系时，不能把将由本地玩家控制的 Pawn 提前标为输入就绪。
		if (Pawn->GetLocalRole() != ROLE_SimulatedProxy && !Pawn->GetController())
		{
			return false;
		}
		// 配置可选；远端玩家/模拟代理/AI 不等待本地输入，监听服务器的本地玩家仍需检查。
		// PS 配对由 PawnInitialization 参与共同屏障；这里不能等待 ASC 初始化，以免互相阻塞。
		if (Data->InputConfig && Pawn->IsLocallyControlled() && !Pawn->IsBotControlled())
		{
			const APlayerController* Controller = GetController<APlayerController>();
			return Controller && Controller->GetLocalPlayer() && Cast<UExtInputComponent>(Pawn->InputComponent);
		}
		return true;
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

void UOmniInputManagerComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	if (CurrentState == ExpSystemTags::TAG_InitState_DataAvailable && DesiredState == ExpSystemTags::TAG_InitState_DataInitialized)
	{
		RefreshInputBind();
	}
}

void UOmniInputManagerComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName == UExpPawnExtensionComponent::NAME_ActorFeatureName
		&& Params.FeatureState == ExpSystemTags::TAG_InitState_DataInitialized)
	{
		CheckDefaultInitialization();
	}
}

void UOmniInputManagerComponent::CheckDefaultInitialization()
{
	ContinueInitStateChain(ExpSystemTags::ComponentStateChain);
}

void UOmniInputManagerComponent::RefreshInputBind()
{
	APawn* Pawn = GetPawn<APawn>();
	APlayerController* Controller = GetController<APlayerController>();
	UExtInputComponent* InputComp = Pawn ? Cast<UExtInputComponent>(Pawn->InputComponent) : nullptr;
	const UExpPawnExtensionComponent* Extension = UExpPawnExtensionComponent::FindPawnExtensionComponent(Pawn);
	const UExpPawnData* Data = Extension ? Extension->GetPawnData<UExpPawnData>() : nullptr;

	// 本地玩家、输入组件和输入配置必须就绪。
	if (!Controller || !Controller->GetLocalPlayer() || !InputComp || !Data || !Data->InputConfig)
	{
		ReleaseInputBind();
		return;
	}

	if (BoundInputComponent == InputComp && BoundController == Controller && BoundInputConfig == Data->InputConfig)
	{
		return;
	}

	ReleaseInputBind();
	BoundInputComponent = InputComp;
	BoundController = Controller;
	BoundInputConfig = Data->InputConfig;

	// 插件按 Triggered/Completed 绑定能力 Tag，与原生输入共用句柄数组。
	InputComp->BindAbilityActions(BoundInputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, BindingHandles);

	// 原生的手动添加到Handle中
	if (const UInputAction* MoveAction = BoundInputConfig->FindNativeInputActionForTag(OmniTags::TAG_InputTag_Move, false))
	{
		BindingHandles.Add(InputComp->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Move).GetHandle());
	}
	if (const UInputAction* LookAction = BoundInputConfig->FindNativeInputActionForTag(OmniTags::TAG_InputTag_Look_Mouse, false))
	{
		BindingHandles.Add(InputComp->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse).GetHandle());
	}

	// 输入绑定完成，通知项目 GF 动作接入。
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(Controller, NAME_BindInputsNow);
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(Pawn, NAME_BindInputsNow);
}

void UOmniInputManagerComponent::ReleaseInputBind()
{
	if (UExtInputComponent* Input = BoundInputComponent.Get())
	{
		Input->RemoveBinds(BindingHandles);
	}

	BindingHandles.Reset();
	BoundInputComponent.Reset();
	BoundController.Reset();
	BoundInputConfig = nullptr;
}

void UOmniInputManagerComponent::Input_Move(const FInputActionValue& Value)
{
	APawn* Pawn = GetPawn<APawn>();
	const AController* Controller = GetController<AController>();
	if (!Pawn || !Controller) return;

	const FVector2D Input = Value.Get<FVector2D>();
	const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
	Pawn->AddMovementInput(MovementRotation.RotateVector(FVector::RightVector), Input.X);
	Pawn->AddMovementInput(MovementRotation.RotateVector(FVector::ForwardVector), Input.Y);
}

void UOmniInputManagerComponent::Input_LookMouse(const FInputActionValue& Value)
{
	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn) return;

	// 鼠标值是每帧增量，不乘 DeltaSeconds；灵敏度与反转由输入 Modifier 配置。
	const FVector2D Input = Value.Get<FVector2D>();
	Pawn->AddControllerYawInput(Input.X);
	Pawn->AddControllerPitchInput(Input.Y);
}

void UOmniInputManagerComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	const UExpPawnExtensionComponent* Extension = UExpPawnExtensionComponent::FindPawnExtensionComponent(GetPawn<APawn>());
	if (UExtAbilitySystemComponent* ASC = Extension ? Extension->GetExtAbilitySystemComponent() : nullptr)
	{
		ASC->AbilityInputTagPressed(InputTag);
	}
}

void UOmniInputManagerComponent::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	const UExpPawnExtensionComponent* Extension = UExpPawnExtensionComponent::FindPawnExtensionComponent(GetPawn<APawn>());
	if (UExtAbilitySystemComponent* ASC = Extension ? Extension->GetExtAbilitySystemComponent() : nullptr)
	{
		ASC->AbilityInputTagReleased(InputTag);
	}
}
