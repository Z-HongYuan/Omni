// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Contra/ContraHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Character/OmniCharacter.h"
#include "Component/ExtHealthComponent.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Contra/ContraPlayerLifeComponent.h"
#include "Contra/ContraRoundComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ContraHUDWidget)

void UContraHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>();
	WidgetTree->RootWidget = Root;
	Status = WidgetTree->ConstructWidget<UTextBlock>();
	FSlateFontInfo Font = Status->GetFont();
	Font.Size = 18;
	Status->SetFont(Font);
	Status->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	Status->SetShadowOffset(FVector2D(1, 1));
	Status->SetShadowColorAndOpacity(FLinearColor::Black);
	Root->AddChildToVerticalBox(Status);
	Restart = WidgetTree->ConstructWidget<UButton>();
	UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>();
	Label->SetText(FText::FromString(TEXT("重新开始 / Restart")));
	Restart->AddChild(Label);
	Restart->OnClicked.AddDynamic(this, &ThisClass::RestartPressed);
	Root->AddChildToVerticalBox(Restart);
}

void UContraHUDWidget::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
	Super::NativeTick(Geometry, DeltaTime);
	const APlayerController* PC = GetOwningPlayer();
	const AOmniCharacter* Pawn = PC ? Cast<AOmniCharacter>(PC->GetPawn()) : nullptr;
	const APlayerState* PS = PC ? PC->PlayerState : nullptr;
	const UContraPlayerLifeComponent* Life = PS ? PS->FindComponentByClass<UContraPlayerLifeComponent>() : nullptr;
	const AGameStateBase* GS = GetWorld()->GetGameState();
	const UContraRoundComponent* Round = GS ? GS->FindComponentByClass<UContraRoundComponent>() : nullptr;
	const EContraRoundPhase Phase = Round ? Round->GetPhase() : EContraRoundPhase::Playing;
	const TCHAR* PhaseText = Phase == EContraRoundPhase::Won ? TEXT("通关") : Phase == EContraRoundPhase::Lost ? TEXT("失败") : TEXT("清场后前往右侧终点");
	Status->SetText(FText::FromString(FString::Printf(TEXT("OMNI CONTRA · 灰盒\n生命 %.0f / %.0f    剩余命数 %d%s\n%s\nA/D 移动 · 空格跳跃 · 鼠标瞄准 · 左键开火"),
	                                                  Pawn ? Pawn->GetHealthComponent()->GetHealth() : 0.0f, Pawn ? Pawn->GetHealthComponent()->GetMaxHealth() : 0.0f,
	                                                  Life ? Life->GetLives() : 0, Life && Life->IsProtected() ? TEXT(" · 复活保护") : TEXT(""), PhaseText)));
	Restart->SetVisibility(Phase == EContraRoundPhase::Playing ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
}

void UContraHUDWidget::RestartPressed()
{
	const APlayerController* PC = GetOwningPlayer();
	const APlayerState* PS = PC ? PC->PlayerState : nullptr;
	if (UContraPlayerLifeComponent* Life = PS ? PS->FindComponentByClass<UContraPlayerLifeComponent>() : nullptr) Life->ServerRequestRestart();
}
