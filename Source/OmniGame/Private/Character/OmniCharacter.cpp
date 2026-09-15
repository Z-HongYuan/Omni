// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Character/OmniCharacter.h"

#include "Character/OmniPawnInitializationComponent.h"
#include "Components/ExpPawnExtensionComponent.h"
#include "System/ExtAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniCharacter)

AOmniCharacter::AOmniCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PawnExtensionComponent = CreateDefaultSubobject<UExpPawnExtensionComponent>(TEXT("PawnExtensionComponent"));
	PawnInitializationComponent = CreateDefaultSubobject<UOmniPawnInitializationComponent>(TEXT("PawnInitializationComponent"));
}

UAbilitySystemComponent* AOmniCharacter::GetAbilitySystemComponent() const
{
	return PawnExtensionComponent->GetExtAbilitySystemComponent();
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
