// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <algorithm>
#include <set>
#include <vector>

#include "../../DataTypes/Highlight.h"
#include "../ExtendedCommon.h"
#include "./DataTypes/EditableGraph.h"
#include "./DataTypes/ExtraLink.h"
#include "./DataTypes/ExtraNode.h"

#define BREAK_LINK

class LoopLinkHandler
{
public:
	static void BreakAllLoops(EditableGraph& editableGraph)
	{
		size_t checkCount = 0;
		std::vector<size_t> linkIdStack;
		std::vector<size_t> nodeIdStack;
		std::set<size_t> visitedNodeIdSet;

		size_t nodeCount = editableGraph.graph.nodeList.size(); // be sure to not check created node
		for (size_t i = 0; i < nodeCount; i++)
		{
			if (contains(visitedNodeIdSet, i)) continue; // visited means already in a visited connected graph
			checkForLoop(i, linkIdStack, nodeIdStack, checkCount, visitedNodeIdSet, editableGraph);
		}
	}

private:
	static void checkForLoop(size_t nodeId,
		std::vector<size_t> linkIdStack,
		std::vector<size_t>& nodeIdStack,
		size_t& checkCount,
		std::set<size_t>& visitedNodeIdSet,
		EditableGraph& editableGraph)
	{
		if (checkCount++ > 50000) throw std::runtime_error("graph too big for arrangement, please consider splitting");
		editableGraph.CheckTimeout();
		// check if node is already in stack
		size_t index;
		if (findIndex(nodeIdStack, nodeId, index))
		{
			// break link since loop starting from index
			breakLink(index, linkIdStack, nodeIdStack, editableGraph);
			return;
		}
		visitedNodeIdSet.insert(nodeId);
		nodeIdStack.push_back(nodeId); // push node to stack
		for (const auto& linkIdList : editableGraph.extraGraph.extraNodeList.at(nodeId).GetLinkIdListList({ELinkFlag::Right}))
			for (const auto& linkId : *linkIdList)
			{
				auto& extraLink = editableGraph.extraGraph.extraLinkList.at(linkId);
				if (extraLink.isBroken) continue;
				linkIdStack.push_back(linkId); // push link to stack
				auto& rightNode = extraLink.GetNode(editableGraph.graph, false);
				checkForLoop(rightNode.id, linkIdStack, nodeIdStack, checkCount, visitedNodeIdSet, editableGraph);
				linkIdStack.pop_back(); // pop link from stack
			}
		nodeIdStack.pop_back(); // pop node from stack
	}

	static void breakLink(size_t index,
		const std::vector<size_t>& linkIdStack,
		const std::vector<size_t>& nodeIdStack,
		EditableGraph& editableGraph)
	{
		auto& extraGraph = editableGraph.extraGraph;

		auto linkToBreakIdPtr = std::max_element(linkIdStack.begin() + index,
			linkIdStack.end(),
			[&extraGraph](const size_t& lhsId, const size_t& rhsId)
			{
				/*
				Better if:
				- already broken
				- between two reroute one output
				- finish with reroute one output
				- nodeDiffX is smaller
				*/
				auto& lhs = extraGraph.extraLinkList.at(lhsId);
				auto& rhs = extraGraph.extraLinkList.at(rhsId);
				if (lhs.bBetweenTwoRerouteOneOutput != rhs.bBetweenTwoRerouteOneOutput)
					return lhs.bBetweenTwoRerouteOneOutput < rhs.bBetweenTwoRerouteOneOutput;
				if (lhs.bFinishWithRerouteOneOutput != rhs.bFinishWithRerouteOneOutput)
					return lhs.bFinishWithRerouteOneOutput < rhs.bFinishWithRerouteOneOutput;
				return lhs.nodeDiffX < rhs.nodeDiffX;
			});
		size_t linkToBreakId = *linkToBreakIdPtr;
		auto& linkToBreak = extraGraph.extraLinkList.at(linkToBreakId);

		size_t linkToBreakIndex = std::distance(linkIdStack.begin(), linkToBreakIdPtr);
		size_t prevLinkToBreakIndex = linkToBreakIndex - 1 < index ? linkIdStack.size() - 1 : linkToBreakIndex - 1;
		size_t nextLinkToBreakIndex = linkToBreakIndex + 1 >= linkIdStack.size() ? index : linkToBreakIndex + 1;
		size_t nextNextLinkToBreakIndex = nextLinkToBreakIndex + 1 >= linkIdStack.size() ? index : nextLinkToBreakIndex + 1;

		// always set as broken if between two reroute one output
		if (linkToBreak.bBetweenTwoRerouteOneOutput)
		{
			// leftParent -> leftReroute -> rightReroute -> rightParent
			// prev -> current -> next -> nextNext

			size_t leftRerouteNodeId = nodeIdStack[linkToBreakIndex];
			size_t rightRerouteNodeId = nodeIdStack[nextLinkToBreakIndex];

			size_t leftParentNodeId = nodeIdStack[prevLinkToBreakIndex];
			size_t rightParentNodeId = nodeIdStack[nextNextLinkToBreakIndex];

			setLoopFamily(leftRerouteNodeId, rightRerouteNodeId, leftParentNodeId, rightParentNodeId, extraGraph);

			linkToBreak.isBroken = true;

			// break also link before and after
			auto& prevLinkToBreak = extraGraph.extraLinkList.at(linkIdStack[prevLinkToBreakIndex]);
			prevLinkToBreak.isBroken = true;
			auto& nextLinkToBreak = extraGraph.extraLinkList.at(linkIdStack[nextLinkToBreakIndex]);
			nextLinkToBreak.isBroken = true;
		}
		else
		{
			// break all created links --> force access to reroute children through ExtraNode::loopChildIdSet
			CREATE_REROUTE(BREAK_LINK, linkToBreakId, true);
			size_t leftRerouteNodeId = extraGraph.extraNodeList.size() - 1;
			auto& extraLinkList = extraGraph.extraLinkList;
			auto& newLinkToBreak = extraLinkList[extraLinkList.size() - 1]; // break last link created

			if (newLinkToBreak.bBetweenTwoRerouteOneOutput)
			{
				// leftParent -> rightReroute -> rightParent
				// current -> next -> nextNext

				size_t rightRerouteNodeId = nodeIdStack[nextLinkToBreakIndex];

				size_t leftParentNodeId = nodeIdStack[linkToBreakIndex];
				size_t rightParentNodeId = nodeIdStack[nextNextLinkToBreakIndex];

				setLoopFamily(leftRerouteNodeId, rightRerouteNodeId, leftParentNodeId, rightParentNodeId, extraGraph);
				editableGraph.CopyNodeComments(leftParentNodeId, leftRerouteNodeId);

				// break also link after
				auto& nextLinkToBreak = extraGraph.extraLinkList.at(linkIdStack[nextLinkToBreakIndex]);
				nextLinkToBreak.isBroken = true;
			}
			else
			{
				// leftParent -> rightParent
				// current -> next

				// break all created links --> force access to reroute children through ExtraNode::loopChildIdSet
				CREATE_REROUTE(BREAK_LINK, newLinkToBreak.id, true);
				size_t rightRerouteNodeId = extraGraph.extraNodeList.size() - 1;

				size_t leftParentNodeId = nodeIdStack[linkToBreakIndex];
				size_t rightParentNodeId = nodeIdStack[nextLinkToBreakIndex];

				setLoopFamily(leftRerouteNodeId, rightRerouteNodeId, leftParentNodeId, rightParentNodeId, extraGraph);
				editableGraph.CopyNodeComments(leftParentNodeId, leftRerouteNodeId);
				editableGraph.CopyNodeComments(rightParentNodeId, rightRerouteNodeId);
			}
		}
	}

	static void setLoopFamily(size_t leftRerouteNodeId,
		size_t rightRerouteNodeId,
		size_t leftParentNodeId,
		size_t rightParentNodeId,
		ExtraGraph& extraGraph)
	{
		auto& leftRerouteExtraNode = extraGraph.extraNodeList.at(leftRerouteNodeId);
		auto& leftParentExtraNode = extraGraph.extraNodeList.at(leftParentNodeId);
		auto& rightRerouteExtraNode = extraGraph.extraNodeList.at(rightRerouteNodeId);
		auto& rightParentExtraNode = extraGraph.extraNodeList.at(rightParentNodeId);

		leftParentExtraNode.loopChildIdSet.insert(leftRerouteNodeId);
		leftRerouteExtraNode.loopBrotherId = rightRerouteNodeId;
		leftRerouteExtraNode.isLoopBrotherPlacedOnRight = true;

		rightParentExtraNode.loopChildIdSet.insert(rightRerouteNodeId);
		rightRerouteExtraNode.loopBrotherId = leftRerouteNodeId;
		rightRerouteExtraNode.isLoopBrotherPlacedOnRight = false;
	}
};
