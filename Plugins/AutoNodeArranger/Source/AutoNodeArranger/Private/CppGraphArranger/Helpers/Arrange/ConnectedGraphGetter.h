// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <algorithm>
#include <set>
#include <vector>

#include "../../DataTypes/Highlight.h"
#include "../ExtendedCommon.h"
#include "./DataTypes/EditableGraph.h"

struct RootInfo
{
	RootInfo() = default;

	RootInfo(size_t nodeId_, bool isRightExecConnected_, const Vector2& pos_) :
		nodeId(nodeId_), isRightExecConnected(isRightExecConnected_), pos(pos_)
	{
	}

	size_t nodeId;
	bool isRightExecConnected;
	Vector2 pos;

	friend bool operator<(const RootInfo& lhs, const RootInfo& rhs)
	{
		/*
			better to change if
			- current is "null"
			- this is more rightExecConnected
			- this is more on top
			- this is more on left
		*/
		if ((lhs.nodeId == static_cast<size_t>(-1)) != (rhs.nodeId == static_cast<size_t>(-1)))
			return lhs.nodeId == static_cast<size_t>(-1);
		if (lhs.isRightExecConnected != rhs.isRightExecConnected) return rhs.isRightExecConnected;
		if (lhs.pos.y != rhs.pos.y) return lhs.pos.y > rhs.pos.y;
		return lhs.pos.x > rhs.pos.x;
	}
};

struct ConnectedGraph
{
	ConnectedGraph() = default;

	ConnectedGraph(size_t rootId_, const std::set<size_t>& nodeIdSet_) : rootId(rootId_), nodeIdSet(nodeIdSet_) {}

	size_t rootId;
	std::set<size_t> nodeIdSet;
};

class ConnectedGraphGetter
{
public:
	static std::vector<ConnectedGraph> GetAllRoots(EditableGraph& editableGraph)
	{
		auto& selectedNodeIdList = editableGraph.graph.selectedNodeIdList;
		std::set<size_t> selectedNodeIdSet(selectedNodeIdList.begin(), selectedNodeIdList.end());

		std::vector<ConnectedGraph> connectedGraphList;

		std::set<size_t> allVisitedNodeIdSet;

		auto arrangeType = editableGraph.graph.graphConfig.arrangeSelectionType;
		if (arrangeType == ECpp_ArrangeSelectionType::AlwaysSelected
			|| (arrangeType == ECpp_ArrangeSelectionType::OneForAll && selectedNodeIdSet.size() > 1))
			isolateSelectedNodes(selectedNodeIdSet, editableGraph);

		for (size_t i = 0; i < editableGraph.graph.nodeList.size(); i++)
		{
			std::set<size_t> visitedNodeIdSet;
			if (contains(allVisitedNodeIdSet, i)) continue; // visited means already in a visited connected graph
			auto& extraNode = editableGraph.extraGraph.extraNodeList.at(i);
			if (extraNode.loopBrotherId != static_cast<size_t>(-1)) // loop reroute node cannot be root
			{
				allVisitedNodeIdSet.insert(i);
				continue;
			}
			RootInfo root(-1, false, Vector2());
			getRoot(root, i, visitedNodeIdSet, editableGraph);
			allVisitedNodeIdSet.insert(visitedNodeIdSet.begin(), visitedNodeIdSet.end());

			// do not arrange graph without selected nodes (if there are selected nodes)
			if (!selectedNodeIdSet.empty() && intersection(visitedNodeIdSet, selectedNodeIdSet).empty()) continue;

			if (root.nodeId != static_cast<size_t>(-1)) connectedGraphList.emplace_back(root.nodeId, visitedNodeIdSet);
			else // if no root found, add node as root (this should not happen)
				connectedGraphList.emplace_back(i, visitedNodeIdSet);
		}
		return connectedGraphList;
	}

private:
	static void getRoot(RootInfo& root, size_t nodeId, std::set<size_t>& visitedNodeIdSet, const EditableGraph& editableGraph)
	{
		editableGraph.CheckTimeout();
		if (contains(visitedNodeIdSet, nodeId)) return;
		visitedNodeIdSet.insert(nodeId); // mark node as visited

		auto& extraGraph = editableGraph.extraGraph;
		auto& extraNode = editableGraph.extraGraph.extraNodeList.at(nodeId);

		bool isLoopRerouteNode = extraNode.loopBrotherId != static_cast<size_t>(-1);
		bool isLeftConnected = extraGraph.IsConnected(nodeId, {ELinkFlag::Left});

		if (!isLoopRerouteNode && !isLeftConnected)
		{
			bool isRightExecConnected = extraGraph.IsConnected(nodeId, {ELinkFlag::RightExec});
			RootInfo newRoot(nodeId, isRightExecConnected, editableGraph.graph.nodeList.at(nodeId).box.Min);
			if (root < newRoot) root = newRoot;
		}

		for (auto& linkIdList : extraNode.GetLinkIdListList({ELinkFlag::All}))
		{
			for (const auto& linkId : *linkIdList)
			{
				auto& extraLink = editableGraph.extraGraph.extraLinkList.at(linkId);
				if (extraLink.isBroken) continue;
				auto& otherNode = extraLink.GetNodeConnectedTo(editableGraph.graph, nodeId);
				getRoot(root, otherNode.id, visitedNodeIdSet, editableGraph);
			}
		}
	}

	static void isolateSelectedNodes(std::set<size_t>& selectedNodeIdSet, EditableGraph& editableGraph)
	{
		// add loop children to arranged nodes
		for (size_t selectedNodeId : selectedNodeIdSet)
		{
			auto& extraSelectedNode = editableGraph.extraGraph.extraNodeList.at(selectedNodeId);
			selectedNodeIdSet.insert(extraSelectedNode.loopChildIdSet.begin(), extraSelectedNode.loopChildIdSet.end());
		}
		// isolate selected nodes
		for (size_t selectedNodeId : selectedNodeIdSet)
		{
			auto& extraSelectedNode = editableGraph.extraGraph.extraNodeList.at(selectedNodeId);
			for (auto& linkIdList : extraSelectedNode.GetLinkIdListList({ELinkFlag::All}))
			{
				for (const auto& linkId : *linkIdList)
				{
					auto& extraLink = editableGraph.extraGraph.extraLinkList.at(linkId);
					if (extraLink.isBroken) continue;
					auto& otherNode = extraLink.GetNodeConnectedTo(editableGraph.graph, selectedNodeId);
					if (!contains(selectedNodeIdSet, otherNode.id))
					{
						extraLink.isBroken = true;
						auto& link = editableGraph.graph.linkList.at(linkId);
						HIGHLIGHT("Isolate", LinkInstruction(link.leftPinId, link.rightPinId, true), linkId);
					}
				}
			}
		}
	}
};
