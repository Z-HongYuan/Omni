// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Core/CosmeticsControllerComponent.h"

#include "LogCustomCosmetics.h"
#include "Core/CosmeticsClientComponent.h"
#include "GameFramework/CheatManagerDefines.h"
#include "Logging/StructuredLog.h"

#if UE_WITH_CHEAT_MANAGER
#include "Data/CosmeticDeveloperSettings.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(CosmeticsControllerComponent)

UCosmeticsControllerComponent::UCosmeticsControllerComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UCosmeticsControllerComponent::BeginPlay()
{
	Super::BeginPlay();

	// 监听 Pawn 更改的事件, 事件位于 AController 中
	if (HasAuthority())
	{
		if (AController* OwningController = GetController<AController>())
		{
			OwningController->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPossessedPawnChanged);

			if (APawn* ControlledPawn = GetPawn<APawn>())
			{
				OnPossessedPawnChanged(nullptr, ControlledPawn);
			}
		}

		ApplyDeveloperSettings();
	}
}

void UCosmeticsControllerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveAllCharacterParts();

	if (HasAuthority())
	{
		if (AController* OwningController = GetController<AController>())
		{
			OwningController->OnPossessedPawnChanged.RemoveAll(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UCosmeticsControllerComponent::AddCharacterPart(const FCharacterPart& NewPart)
{
	UE_LOGFMT(LogCustomCosmetics, Display, "Controller AddCharacterPart {0} to {1}", NewPart.PartClass->GetName(), NewPart.SocketName);

	AddCharacterPartInternal(NewPart, ECharacterPartSource::Natural);
}

void UCosmeticsControllerComponent::RemoveCharacterPart(const FCharacterPart& PartToRemove)
{
	for (auto EntryIt = CharacterParts.CreateIterator(); EntryIt; ++EntryIt)
	{
		if (EntryIt->Part == PartToRemove)
		{
			if (UCosmeticsClientComponent* PawnCustomizer = GetPawnCustomizer())
			{
				UE_LOGFMT(LogCustomCosmetics, Display, "Controller RemoveCharacterPart {0} Index {1}", EntryIt->Part.PartClass->GetName(), EntryIt.GetIndex());
				PawnCustomizer->RemoveCharacterPart(EntryIt->Handle);
			}

			EntryIt.RemoveCurrent();
			break;
		}
	}
}

void UCosmeticsControllerComponent::RemoveAllCharacterParts()
{
	if (UCosmeticsClientComponent* PawnCustomizer = GetPawnCustomizer())
	{
		UE_LOGFMT(LogCustomCosmetics, Display, "Controller RemoveAllCharacterPart Num {0}", CharacterParts.Num());

		for (FCharacterPartControllerEntry& Entry : CharacterParts)
		{
			PawnCustomizer->RemoveCharacterPart(Entry.Handle);
		}
	}

	CharacterParts.Reset();
}

void UCosmeticsControllerComponent::ApplyDeveloperSettings()
{
#if UE_WITH_CHEAT_MANAGER
	const UCosmeticDeveloperSettings* Settings = GetDefault<UCosmeticDeveloperSettings>();

	// 是否需要覆盖
	const bool bSuppressNaturalParts = (Settings->CheatMode == ECosmeticCheatMode::ReplaceParts) && (Settings->CheatCharacterParts.Num() > 0);
	SetSuppressionOnNaturalParts(bSuppressNaturalParts);

	// 移除开发者设置添加的任何内容，然后重新添加
	UCosmeticsClientComponent* PawnCustomizer = GetPawnCustomizer();
	for (auto It = CharacterParts.CreateIterator(); It; ++It)
	{
		if (It->Source == ECharacterPartSource::AppliedViaDeveloperSettingsCheat)
		{
			if (PawnCustomizer != nullptr)
			{
				PawnCustomizer->RemoveCharacterPart(It->Handle);
			}
			It.RemoveCurrent();
		}
	}

	// 添加 新外观
	for (const FCharacterPart& PartDesc : Settings->CheatCharacterParts)
	{
		AddCharacterPartInternal(PartDesc, ECharacterPartSource::AppliedViaDeveloperSettingsCheat);
	}
#endif
}

UCosmeticsClientComponent* UCosmeticsControllerComponent::GetPawnCustomizer() const
{
	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		return ControlledPawn->FindComponentByClass<UCosmeticsClientComponent>();
	}
	return nullptr;
}

void UCosmeticsControllerComponent::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	// 从旧的 Pawn 中移除所有外观
	if (UCosmeticsClientComponent* OldCustomizer = OldPawn ? OldPawn->FindComponentByClass<UCosmeticsClientComponent>() : nullptr)
	{
		for (FCharacterPartControllerEntry& Entry : CharacterParts)
		{
			OldCustomizer->RemoveCharacterPart(Entry.Handle);
			Entry.Handle.Reset();
		}
	}

	// 向新 Pawn 中添加所有外观
	if (UCosmeticsClientComponent* NewCustomizer = NewPawn ? NewPawn->FindComponentByClass<UCosmeticsClientComponent>() : nullptr)
	{
		for (FCharacterPartControllerEntry& Entry : CharacterParts)
		{
			// 如果已经存在，不要重新添加，否则可能会被旧 Pawn 调用
			if (!Entry.Handle.IsValid() && Entry.Source != ECharacterPartSource::NaturalSuppressedViaCheat)
			{
				Entry.Handle = NewCustomizer->AddCharacterPart(Entry.Part);
			}
		}
	}
}

void UCosmeticsControllerComponent::AddCharacterPartInternal(const FCharacterPart& NewPart, ECharacterPartSource Source)
{
	FCharacterPartControllerEntry& NewEntry = CharacterParts.AddDefaulted_GetRef();
	NewEntry.Part = NewPart;
	NewEntry.Source = Source;

	if (UCosmeticsClientComponent* PawnCustomizer = GetPawnCustomizer())
	{
		if (NewEntry.Source != ECharacterPartSource::NaturalSuppressedViaCheat)
		{
			NewEntry.Handle = PawnCustomizer->AddCharacterPart(NewPart);
		}
	}
}

void UCosmeticsControllerComponent::AddCheatPart(const FCharacterPart& NewPart, bool bSuppressNaturalParts)
{
#if UE_WITH_CHEAT_MANAGER
	SetSuppressionOnNaturalParts(bSuppressNaturalParts);
	AddCharacterPartInternal(NewPart, ECharacterPartSource::AppliedViaCheatManager);
#endif
}

void UCosmeticsControllerComponent::ClearCheatParts()
{
#if UE_WITH_CHEAT_MANAGER
	UCosmeticsClientComponent* PawnCustomizer = GetPawnCustomizer();

	// 移除所有由作弊添加的外观
	for (auto It = CharacterParts.CreateIterator(); It; ++It)
	{
		if (It->Source == ECharacterPartSource::AppliedViaCheatManager)
		{
			if (PawnCustomizer != nullptr)
			{
				PawnCustomizer->RemoveCharacterPart(It->Handle);
			}
			It.RemoveCurrent();
		}
	}

	ApplyDeveloperSettings();
#endif
}

void UCosmeticsControllerComponent::SetSuppressionOnNaturalParts(bool bSuppressed)
{
#if UE_WITH_CHEAT_MANAGER
	UCosmeticsClientComponent* PawnCustomizer = GetPawnCustomizer();

	for (FCharacterPartControllerEntry& Entry : CharacterParts)
	{
		if ((Entry.Source == ECharacterPartSource::Natural) && bSuppressed)
		{
			// 覆盖
			if (PawnCustomizer != nullptr)
			{
				PawnCustomizer->RemoveCharacterPart(Entry.Handle);
				Entry.Handle.Reset();
			}
			Entry.Source = ECharacterPartSource::NaturalSuppressedViaCheat;
		}
		else if ((Entry.Source == ECharacterPartSource::NaturalSuppressedViaCheat) && !bSuppressed)
		{
			// 不覆盖
			if (PawnCustomizer != nullptr)
			{
				Entry.Handle = PawnCustomizer->AddCharacterPart(Entry.Part);
			}
			Entry.Source = ECharacterPartSource::Natural;
		}
	}
#endif
}
