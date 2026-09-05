// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "ANA_GraphAnimator.h"

#include "../../Config/ANA_EditorConfig.h"
#include "../ANA_InputProcessor.h"
#include "../Arranger/ANA_InstructionsExecutor.h"

void ANA_GraphAnimator::AnimateGraphInstruction(ANA_GraphExport& graphExport, const GraphInstruction& graphInstruction)
{
	auto& graphConfig = graphExport.cppGraph.graphConfig;
	FString connectModeString = graphConfig.bCompact ? "Compact" : graphConfig.bCenterize ? "Center" : "Straight";
	GEditor->BeginTransaction(*FString("AutoNodeArranger"),
		FText::FromString("Arrange graph " + connectModeString),
		graphExport.graphPanel->GetGraphObj());
	graphExport.graphPanel->GetGraphObj()->Modify();

	ANA_InstructionsExecutor::ExecuteGraphInstructionWithoutNode(graphExport, graphInstruction);
	float alpha = 0.f;
	float speed = UANA_EditorConfig::Get()->bInstantArrange ? UANA_EditorConfig::instantSpeed : UANA_EditorConfig::normalSpeed;

	ANA_InputProcessor::tickEventId++;
	ANA_InputProcessor::keyDownEventId++;
	ANA_InputProcessor::Get().tickDispatcher.Add(ANA_InputProcessor::tickEventId,
		[tickEventId_ = ANA_InputProcessor::tickEventId,
			keyDownEventId_ = ANA_InputProcessor::keyDownEventId,
			alpha,
			speed,
			graphExport,
			graphInstruction](const float DeltaTime) mutable
		{
			alpha = FMath::Clamp(alpha + DeltaTime * speed, 0.f, 1.f);
			float lerp = FMath::InterpEaseInOut(0.f, 1.f, alpha, UANA_EditorConfig::exponent);
			ANA_InstructionsExecutor::ExecuteNodeInstructions(graphExport, graphInstruction, lerp);

			if (alpha >= 1.f)
			{
				ANA_InputProcessor::Get().tickDispatcher.Remove(tickEventId_);
				ANA_InputProcessor::Get().keyDownDispatcher.Remove(keyDownEventId_);
				GEditor->EndTransaction();
			}
		});
	ANA_InputProcessor::Get().keyDownDispatcher.Add(ANA_InputProcessor::keyDownEventId,
		[tickEventId_ = ANA_InputProcessor::tickEventId, keyDownEventId_ = ANA_InputProcessor::keyDownEventId](
			const FKeyEvent& InKeyEvent)
		{
			if (InKeyEvent.IsControlDown() && InKeyEvent.GetKey() == EKeys::Z)
			{
				ANA_InputProcessor::Get().tickDispatcher.Remove(tickEventId_);
				ANA_InputProcessor::Get().keyDownDispatcher.Remove(keyDownEventId_);
				auto transactionId = GEditor->EndTransaction();
				GEditor->CancelTransaction(transactionId);
			}
		});
}
