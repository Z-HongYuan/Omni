// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "../../CppGraphArranger/DataTypes/Graph/CppGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraphNode_Comment.h"
#include "SGraphPanel.h"

struct ANA_GraphExport
{
	explicit ANA_GraphExport(SGraphPanel* graphPanel_) : graphPanel(graphPanel_) {}

	SGraphPanel* graphPanel;
	CppGraph cppGraph;
	TArray<UEdGraphNode*> graphNodeList;
	TArray<UEdGraphNode_Comment*> graphCommentList;
	TArray<UEdGraphPin*> graphPinList;
};
