// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "../../CppGraphArranger/DataTypes/Instruction/GraphInstruction.h"
#include "../../CppGraphArranger/DataTypes/Instruction/SelectInstruction.h"
#include "../DataTypes/ANA_GraphExport.h"

class ANA_InstructionsExecutor
{
private:
	ANA_InstructionsExecutor() = delete;

public:
	static void ExecuteGraphInstructionWithoutNode(ANA_GraphExport& graphExport, const GraphInstruction& instruction);

	static void ExecuteNodeInstructions(const ANA_GraphExport& graphExport, const GraphInstruction& instruction, double alpha);

	static void ExecuteSelectInstruction(const ANA_GraphExport& graphExport, const SelectInstruction& instruction);
};
