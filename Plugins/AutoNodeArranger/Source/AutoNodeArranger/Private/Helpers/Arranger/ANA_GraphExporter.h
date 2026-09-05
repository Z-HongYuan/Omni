// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "../DataTypes/ANA_GraphExport.h"
#include "EdGraph/EdGraphNode.h"
#include "K2Node_Knot.h"
#include "MaterialGraphNode_Knot.h"

class ANA_GraphExporter
{
private:
	ANA_GraphExporter() = delete;

public:
	// returns true if export succeed
	static bool ExportGraph(const CppGraphConfig& graphConfig, ANA_GraphExport& graphExport, bool isArranging);
};
