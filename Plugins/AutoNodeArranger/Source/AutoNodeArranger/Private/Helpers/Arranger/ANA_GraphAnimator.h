// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "../../CppGraphArranger/DataTypes/Instruction/GraphInstruction.h"
#include "../DataTypes/ANA_GraphExport.h"

class ANA_GraphAnimator
{
private:
	ANA_GraphAnimator() = delete;

public:
	static void AnimateGraphInstruction(ANA_GraphExport& graphExport, const GraphInstruction& graphInstruction);
};
