// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Character/OmniCharacter.h"

#include "Character/OmniCMC.h"
#include "Character/OmniPawnInitializationComponent.h"
#include "Component/ExtHealthComponent.h"
#include "Component/ExtDeathComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/ExpPawnExtensionComponent.h"
#include "System/ExtAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniCharacter)

AOmniCharacter::AOmniCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UOmniCMC>(ACharacter::CharacterMovementComponentName))
{
	PawnExtensionComponent = CreateDefaultSubobject<UExpPawnExtensionComponent>(TEXT("PawnExtensionComponent"));

	PawnInitializationComponent = CreateDefaultSubobject<UOmniPawnInitializationComponent>(TEXT("PawnInitializationComponent"));
	// 主项目负责接线，生命插件只认识传入的 ASC。
	PawnExtensionComponent->CallOrRegister_AbilitySystemInitialized(FSimpleDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
	PawnExtensionComponent->Register_AbilitySystemUninitialized(FSimpleDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));

	HealthComponent = CreateDefaultSubobject<UExtHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->OnOutOfHealth.AddDynamic(this, &ThisClass::OnOutOfHealth);

	DeathComponent = CreateDefaultSubobject<UExtDeathComponent>(TEXT("DeathComponent"));
	DeathComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
}

void AOmniCharacter::OnAbilitySystemInitialized()
{
	DeathComponent->InitializeWithAbilitySystem(GetExtAbilitySystemComponent());
	HealthComponent->InitializeWithAbilitySystem(GetExtAbilitySystemComponent());
}

void AOmniCharacter::OnAbilitySystemUninitialized()
{
	HealthComponent->UninitializeFromAbilitySystem();
	DeathComponent->UninitializeFromAbilitySystem();
}

void AOmniCharacter::OnOutOfHealth(UExtHealthComponent* Component, float OldValue, float NewValue, AActor* InstigatorActor)
{
	DeathComponent->StartDeath();
}

void AOmniCharacter::OnDeathStarted(AActor* OwningActor)
{
	StopJumping();
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AOmniCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	OnAbilitySystemUninitialized();

	// 在父类解除占有前通知旧 Avatar 的消费者，避免 PC 先清空 Avatar 后漏掉注销广播。
	PawnExtensionComponent->UninitializeAbilitySystem();

	Super::EndPlay(EndPlayReason);
}

UExtAbilitySystemComponent* AOmniCharacter::GetExtAbilitySystemComponent() const
{
	return PawnExtensionComponent ? PawnExtensionComponent->GetExtAbilitySystemComponent() : nullptr;
}

UAbilitySystemComponent* AOmniCharacter::GetAbilitySystemComponent() const
{
	return GetExtAbilitySystemComponent();
}

void AOmniCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	PawnExtensionComponent->HandleControllerChanged();
}

void AOmniCharacter::UnPossessed()
{
	Super::UnPossessed();

	// 沿用 Lyra：这里只刷新控制关系，完整 ASC 清理由 PawnExtension 的生命周期负责。
	PawnExtensionComponent->HandleControllerChanged();
}

void AOmniCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	PawnExtensionComponent->HandleControllerChanged();
}

void AOmniCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	PawnExtensionComponent->HandlePlayerStateReplicated();
}

void AOmniCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PawnExtensionComponent->SetupPlayerInputComponent();
}
