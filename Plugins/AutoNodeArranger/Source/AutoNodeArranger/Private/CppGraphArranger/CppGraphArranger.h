// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "./DataTypes/Graph/CppGraph.h"
#include "./Helpers/Arrange/ConnectedGraphGetter.h"
#include "./Helpers/Arrange/DataTypes/EditableGraph.h"
#include "./Helpers/Arrange/LineOverlapHandler.h"
#include "./Helpers/Arrange/LoopLinkHandler.h"
#include "./Helpers/Arrange/NodeAdjuster.h"
#include "./Helpers/Arrange/NodePlacerX.h"
#include "./Helpers/Arrange/NodePlacerY.h"
#include "./Helpers/Arrange/RerouteCleaner.h"
#include "./Helpers/Arrange/ReroutePlacerY.h"

class CppGraphArranger
{
public:
	static GraphInstruction ArrangeGraph(const CppGraph& graph)
	{
		globalHiglights.clear();

		if (!graph.graphConfig.bUseArrangement) return GraphInstruction();

		EditableGraph editableGraph(graph);
		editableGraph.InitializeTimeout();

		LoopLinkHandler::BreakAllLoops(editableGraph);
		auto connectedGraphList = ConnectedGraphGetter::GetAllRoots(editableGraph);
		for (const auto& connectedGraph : connectedGraphList) RerouteCleaner::CleanReroutes(connectedGraph.rootId, editableGraph);
		editableGraph.SortLink();
		editableGraph.UpdateAllCommentInfo();

		for (const auto& connectedGraph : connectedGraphList)
		{
			NodePlacerX::PlaceGraphInX(connectedGraph.rootId, editableGraph);
			NodePlacerY::PlaceGraphInY(connectedGraph.rootId, editableGraph);
			if (NodeAdjuster::AdjustNodes(connectedGraph.nodeIdSet, editableGraph))
			{
				NodePlacerX::PlaceGraphInX(connectedGraph.rootId, editableGraph);
				NodePlacerY::PlaceGraphInY(connectedGraph.rootId, editableGraph);
			}
			if (graph.graphConfig.bUseReroutePlacerY) ReroutePlacerY::PlaceRerouteInY(connectedGraph.rootId, editableGraph);
			LineOverlapHandler::PlaceLines(connectedGraph.rootId, editableGraph);
		}

		editableGraph.SetAllCommentInstructions();
		if (graph.graphConfig.bUseReroutePlacerY) editableGraph.ReconciliateRerouteInstructions();
		editableGraph.SetMergedNodeInstructionList();

		return editableGraph.GetGraphInstruction();
	}
};
