// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Player/OmniCharacterBase.h"

#include "Components/ExperiencePawnExtensionComponent.h"
#include "System/CustomAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniCharacterBase)

AOmniCharacterBase::AOmniCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 注意：UExperiencePawnExtensionComponent 构造里已经 SetIsReplicatedByDefault(true)，这里不用再设一次
	PawnExtensionComponent = CreateDefaultSubobject<UExperiencePawnExtensionComponent>(TEXT("PawnExtensionComponent"));

	// 注册 ASC 就绪 / 注销的回调
	// CallOrRegister_ 版本：若此刻 ASC 已经就绪会立刻回调一次，不会漏掉
	PawnExtensionComponent->CallOrRegister_AbilitySystemInitialized(
		FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
	PawnExtensionComponent->Register_AbilitySystemUninitialized(
		FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));
}

void AOmniCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	PawnExtensionComponent->HandleControllerChanged();
}

void AOmniCharacterBase::UnPossessed()
{
	Super::UnPossessed();

	PawnExtensionComponent->HandleControllerChanged();
}

void AOmniCharacterBase::OnRep_Controller()
{
	Super::OnRep_Controller();

	PawnExtensionComponent->HandleControllerChanged();
}

void AOmniCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	PawnExtensionComponent->HandlePlayerStateReplicated();
}

void AOmniCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PawnExtensionComponent->SetupPlayerInputComponent();
}

UCustomAbilitySystemComponent* AOmniCharacterBase::GetCustomAbilitySystemComponent() const
{
	return PawnExtensionComponent ? PawnExtensionComponent->GetCustomAbilitySystemComponent() : nullptr;
}

UAbilitySystemComponent* AOmniCharacterBase::GetAbilitySystemComponent() const
{
	return GetCustomAbilitySystemComponent();
}

void AOmniCharacterBase::OnAbilitySystemInitialized()
{
	// 空实现，供派生类重载
	// 派生类可在此初始化生命值组件、运动状态相关的 GameplayTag 等
}

void AOmniCharacterBase::OnAbilitySystemUninitialized()
{
	// 空实现，供派生类重载
}
