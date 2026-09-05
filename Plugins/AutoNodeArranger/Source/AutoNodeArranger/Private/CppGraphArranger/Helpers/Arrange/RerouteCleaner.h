// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <list>
#include <set>

#include "../../DataTypes/Highlight.h"
#include "./NodePlacerCommon.h"

#define CLEAN_REROUTE

class RerouteCleaner
{
public:
	static void CleanReroutes(size_t root, EditableGraph& editableGraph)
	{
		std::set<size_t> visitedNodeIdSet;
		cleanReroute(root, visitedNodeIdSet, editableGraph);
	}

private:
	static void cleanReroute(size_t nodeId, std::set<size_t>& visitedNodeIdSet, EditableGraph& editableGraph)
	{
		editableGraph.CheckTimeout();
		if (contains(visitedNodeIdSet, nodeId)) return;
		visitedNodeIdSet.insert(nodeId);

		auto& graph = editableGraph.graph;
		auto& extraGraph = editableGraph.extraGraph;
		auto& extraNode = extraGraph.extraNodeList.at(nodeId);
		auto& node = graph.nodeList.at(nodeId);
		if (node.isRerouteNode)
		{
			TEMP_DELETE_REROUTE(CLEAN_REROUTE, nodeId);
			return;
		}

		for (auto linkIdList : extraNode.GetLinkIdListList({ELinkFlag::All}))
		{
			for (size_t linkId : *linkIdList)
			{
				auto& extraLink = extraGraph.extraLinkList.at(linkId);
				auto& otherNode = extraLink.GetNodeConnectedTo(graph, nodeId);
				if (extraLink.isBroken) continue;
				cleanReroute(otherNode.id, visitedNodeIdSet, editableGraph);
			}
		}
	}
};
