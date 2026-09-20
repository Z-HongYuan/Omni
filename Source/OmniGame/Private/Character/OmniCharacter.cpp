// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Character/OmniCharacter.h"

#include "Character/OmniCMC.h"
#include "Character/OmniPawnInitializationComponent.h"
#include "Component/ExtHealthComponent.h"
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
	HealthComponent = CreateDefaultSubobject<UExtHealthComponent>(TEXT("HealthComponent"));

	// 主项目负责接线，生命插件只认识传入的 ASC。
	PawnExtensionComponent->CallOrRegister_AbilitySystemInitialized(FSimpleDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
	PawnExtensionComponent->Register_AbilitySystemUninitialized(FSimpleDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));
}

void AOmniCharacter::OnAbilitySystemInitialized()
{
	HealthComponent->InitializeWithAbilitySystem(GetExtAbilitySystemComponent());
}

void AOmniCharacter::OnAbilitySystemUninitialized()
{
	HealthComponent->UninitializeFromAbilitySystem();
}

void AOmniCharacter::Reset()
{
	// 相对 Lyra 增加显式服务器约束，供玩法蓝图调用；客户端随角色销毁复制退场。
	if (!HasAuthority()) return;

	DisableMovementAndCollision();
	K2_OnReset();
	UninitAndDestroy();
}

void AOmniCharacter::DisableMovementAndCollision()
{
	StopJumping();
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AOmniCharacter::UninitAndDestroy()
{
	// 与 Lyra 的顺序差异：先注销，再解除控制；PC 的 OnUnPossess 会提前清空 Avatar。
	if (UExtAbilitySystemComponent* ASC = GetExtAbilitySystemComponent())
	{
		// ASC 可能已交给新 Pawn，旧角色不能清理新 Avatar 的能力系统。
		if (ASC->GetAvatarActor() == this)
		{
			PawnExtensionComponent->UninitializeAbilitySystem();
		}
	}

	if (HasAuthority())
	{
		DetachFromControllerPendingDestroy();
		SetLifeSpan(0.1f);
	}

	SetActorHiddenInGame(true);
}

UExtAbilitySystemComponent* AOmniCharacter::GetExtAbilitySystemComponent() const
{
	return PawnExtensionComponent ? PawnExtensionComponent->GetExtAbilitySystemComponent() : nullptr;
}

UAbilitySystemComponent* AOmniCharacter::GetAbilitySystemComponent() const
{
	return GetExtAbilitySystemComponent();
}

void AOmniCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->GetOwnedGameplayTags(TagContainer);
	}
	else
	{
		TagContainer.Reset();
	}
}

bool AOmniCharacter::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	return ASC ? ASC->HasMatchingGameplayTag(TagToCheck) : false;
}

bool AOmniCharacter::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	return ASC ? ASC->HasAllMatchingGameplayTags(TagContainer) : false;
}

bool AOmniCharacter::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	return ASC ? ASC->HasAnyMatchingGameplayTags(TagContainer) : false;
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
