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

	// 由权威端监听 Pawn 切换，并把控制器保存的外观应用到当前 Pawn。
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

	// 只有替换模式且存在调试部件时才抑制常规部件。
	const bool bSuppressNaturalParts = (Settings->CheatMode == ECosmeticCheatMode::ReplaceParts) && (Settings->CheatCharacterParts.Num() > 0);
	SetSuppressionOnNaturalParts(bSuppressNaturalParts);

	// 先移除上次开发者设置生成的部件，避免重复叠加。
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

	// 应用当前开发者设置中的部件。
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
	// 从旧 Pawn 移除本组件管理的部件，并清空对应句柄。
	if (UCosmeticsClientComponent* OldCustomizer = OldPawn ? OldPawn->FindComponentByClass<UCosmeticsClientComponent>() : nullptr)
	{
		for (FCharacterPartControllerEntry& Entry : CharacterParts)
		{
			OldCustomizer->RemoveCharacterPart(Entry.Handle);
			Entry.Handle.Reset();
		}
	}

	// 将未受调试抑制的外观应用到新 Pawn。
	if (UCosmeticsClientComponent* NewCustomizer = NewPawn ? NewPawn->FindComponentByClass<UCosmeticsClientComponent>() : nullptr)
	{
		for (FCharacterPartControllerEntry& Entry : CharacterParts)
		{
			// 仅创建尚未应用的部件，避免重复添加有效句柄对应的外观。
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

	// 只移除 CheatManager 命令添加的部件，开发者设置稍后重新应用。
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
			// 移除当前表现并保留常规部件描述，供取消抑制时恢复。
			if (PawnCustomizer != nullptr)
			{
				PawnCustomizer->RemoveCharacterPart(Entry.Handle);
				Entry.Handle.Reset();
			}
			Entry.Source = ECharacterPartSource::NaturalSuppressedViaCheat;
		}
		else if ((Entry.Source == ECharacterPartSource::NaturalSuppressedViaCheat) && !bSuppressed)
		{
			// 恢复此前被调试覆盖抑制的常规部件。
			if (PawnCustomizer != nullptr)
			{
				Entry.Handle = PawnCustomizer->AddCharacterPart(Entry.Part);
			}
			Entry.Source = ECharacterPartSource::Natural;
		}
	}
#endif
}
