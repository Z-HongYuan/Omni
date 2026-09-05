// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "../../CppGraphArranger/DataTypes/Instruction/GraphInstruction.h"
#include "../_CustomObjects/Graph/Graph.h"

#include "ExtendedComment.h"
#include "ExtendedNode.h"
#include "ExtendedPin.h"

#include "../../CppGraphArranger/Helpers/ExtendedCommon.h"

#include "../ConnectedGraph.h"
#include "../Lib/LoopConnectionLib.h"

struct ExtendedGraph : OldCppGraph<ExtendedPin, ExtendedNode, ExtendedComment>
{
	// ======== ATTRIBUTES ========

	GraphInstruction instructionGlobalContainer;

	std::set<ExtendedComment*> connectedToNodeSelectionCommentSet;
	std::vector<OldConnectedGraph> connectedGraphList;

	// ======== METHODS ========

	void doAll()
	{
		ExtendedNode::InitializeTimeout();
		computeGraphExtension();
		breakAllLoops();
		computeConnectedGraphList();
		defineCommentKinship();
		doArrangement();
	}

	void computeGraphExtension()
	{
		ExtendedNode::setNodeStaticAttributes(&instructionGlobalContainer.allNodeInstructionList, &graphConfig);
		for (auto& node : nodeList)
		{
			node.updatePinIDListArray();
			node.updateConnectionListArray();
		}
		for (auto& comment : commentList) comment.computeCommentExtensionForCommentAndNode(nodeList, commentList);
		for (auto& node : nodeList) node.computeNodeExtensionForPin(pinList);
		for (auto& node : nodeList) node.computeNodeExtension(pinList, nodeList);
		if (graphConfig.GetIsCompactBlueprintGraph())
		{
			for (auto& node : nodeList) node.computeCompactNodeExtension();
		}
		for (auto& comment : commentList)
			comment.computeCommentExtension(graphConfig.GetIsAI_Graph(), graphConfig.GetIsMaterialOrSoundGraph());
	}

	void breakAllLoops()
	{
		std::set<ExtendedNode*> connectedToNodeSelectionNodeSet;
		ExtendedNode* globalRootNode = nullptr; // not used
		for (auto selectedNodeID : selectedNodeIDList)
			nodeList[selectedNodeID].UpdateConnectedNodeSet(connectedToNodeSelectionNodeSet, globalRootNode);
		LoopConnectionLib loopConnectionLib = LoopConnectionLib(&graphConfig,
			&instructionGlobalContainer.rerouteInstructionList,
			&instructionGlobalContainer.linkInstructionList,
			&connectedToNodeSelectionNodeSet,
			&pinList,
			&nodeList);
		if (graphConfig.GetIsBlueprintGraph())
		{
			loopConnectionLib.AutoGenerateRerouteLoopNode();
			loopConnectionLib.UpdateRerouteLoopNodeFamily();
		}
		loopConnectionLib.BreakAllLoops();
	}

	void computeConnectedGraphList()
	{
		std::set<size_t> selectedNodeIdSet(selectedNodeIDList.begin(), selectedNodeIDList.end());
		auto arrangeType = graphConfig.arrangeSelectionType;

		if (arrangeType == ECpp_ArrangeSelectionType::AlwaysSelected
			|| (arrangeType == ECpp_ArrangeSelectionType::OneForAll && selectedNodeIdSet.size() > 1))
			isolateSelectedNodes(selectedNodeIdSet);

		std::set<ExtendedNode*> connectedToNodeSelectionNodeSet;
		for (auto selectedNodeID : selectedNodeIDList)
		{
			if (contains(connectedToNodeSelectionNodeSet, &nodeList[selectedNodeID])) continue;
			connectedGraphList.emplace_back(&nodeList[selectedNodeID], commentList);
			connectedToNodeSelectionNodeSet.insert(
				connectedGraphList.back().connectedNodeSet.begin(), connectedGraphList.back().connectedNodeSet.end());
			connectedToNodeSelectionCommentSet.insert(
				connectedGraphList.back().connectedCommentSet.begin(), connectedGraphList.back().connectedCommentSet.end());
		}
	}

	void isolateSelectedNodes(std::set<size_t>& selectedNodeIdSet)
	{
		// add loop children to arranged nodes
		for (size_t selectedNodeId : selectedNodeIdSet)
		{
			auto extendedSelectedNode = nodeList[selectedNodeId];
			if (extendedSelectedNode.rerouteLoopNodeChild)
				selectedNodeIdSet.insert(extendedSelectedNode.rerouteLoopNodeChild->ID);
			if (extendedSelectedNode.rerouteLoopNodeBrother)
				selectedNodeIdSet.insert(extendedSelectedNode.rerouteLoopNodeBrother->ID);
		}
		// isolate selected nodes
		for (size_t selectedNodeId : selectedNodeIdSet)
		{
			auto& extendedSelectedNode = nodeList[selectedNodeId];
			for (auto& connectionList : extendedSelectedNode.allConnectionListArray)
			{
				for (auto& connection : *connectionList)
				{
					if (!contains(selectedNodeIdSet, connection.connectedToNode->ID))
					{
						auto linkInstruction = connection.breakConnection();
						HIGHLIGHT("Isolate", linkInstruction);
					}
				}
			}
		}
	}

	void defineCommentKinship()
	{
		for (auto& connectedToNodeSelectionComment : connectedToNodeSelectionCommentSet)
		{
			instructionGlobalContainer.commentInstructionList.emplace_back(connectedToNodeSelectionComment->ID);
			instructionGlobalContainer.commentInstructionList.back().nodeIdUnderCommentList
				= connectedToNodeSelectionComment->insideNodeIDList;
			instructionGlobalContainer.commentInstructionList.back().commentIdUnderCommentList
				= connectedToNodeSelectionComment->insideCommentIDList;
		}
	}

	void doArrangement()
	{
		for (auto& connectedGraph : connectedGraphList) connectedGraph.globalRootNode->PlaceAllNodes();

		if (graphConfig.bGroupAllConnectedGraph && connectedGraphList.size() >= 2)
		{
			for (auto& connectedGraph : connectedGraphList)
			{
				connectedGraph.updateConnectedGraphBox();
				connectedGraph.updateCommentConnectedGraphBox(graphConfig.commentSpacing);
			}
			if (graphConfig.GetIsAI_Graph())
			{
				std::sort(connectedGraphList.begin(),
					connectedGraphList.end(),
					[](const OldConnectedGraph& lhs, const OldConnectedGraph& rhs)
					{
						return lhs.globalRootNode->pos.x != rhs.globalRootNode->pos.x
								   ? lhs.globalRootNode->pos.x < rhs.globalRootNode->pos.x
								   : lhs.globalRootNode->pos.y < rhs.globalRootNode->pos.y;
					});
			}
			else
			{
				std::sort(connectedGraphList.begin(),
					connectedGraphList.end(),
					[](const OldConnectedGraph& lhs, const OldConnectedGraph& rhs)
					{
						return lhs.globalRootNode->pos.y != rhs.globalRootNode->pos.y
								   ? lhs.globalRootNode->pos.y < rhs.globalRootNode->pos.y
								   : lhs.globalRootNode->pos.x < rhs.globalRootNode->pos.x;
					});
			}

			Vector2 globalConnectedGraphMinPos = connectedGraphList[0].getMinPos(graphConfig.spacing, graphConfig.commentSpacing);
			Vector2 minPos = globalConnectedGraphMinPos;
			for (int i = 1; i < connectedGraphList.size(); ++i)
			{
				if (intersection(connectedGraphList[i].connectedCommentSet, connectedGraphList[0].connectedCommentSet).empty())
					continue;
				minPos = minPos.minWith(connectedGraphList[i].getMinPos(graphConfig.spacing, graphConfig.commentSpacing));
			}
			connectedGraphList[0].moveConnectedGraph(graphConfig.GetIsAI_Graph()
														 ? Vector2(0., minPos.y - globalConnectedGraphMinPos.y)
														 : Vector2(minPos.x - globalConnectedGraphMinPos.x, 0.));
			connectedGraphList[0].isPlaced = true;

			std::vector<OldConnectedGraph*> toPlaceConnectedGraphStack;
			for (int i = connectedGraphList.size() - 1; i >= 1; --i) toPlaceConnectedGraphStack.push_back(&connectedGraphList[i]);

			OldConnectedGraph* lastConnectedGraphPlaced = &connectedGraphList[0];
			const Box* lastBox = lastConnectedGraphPlaced->getConnectedGraphBox();

			while (!toPlaceConnectedGraphStack.empty())
			{
				ExtendedNode::CheckTimeout();
				OldConnectedGraph* toPlaceConnectedGraph = toPlaceConnectedGraphStack.back();

				// get next to place
				OldConnectedGraph* nextToPlace = nullptr;
				OldConnectedGraph::CommentRelativeInfo relativeInfoNextToPlace;
				for (auto& connectedGraph : connectedGraphList)
				{
					if (connectedGraph.isPlaced) continue;
					OldConnectedGraph::CommentRelativeInfo relativeInfo
						= OldConnectedGraph::CommentRelativeInfo(connectedGraph, *toPlaceConnectedGraph);
					if (relativeInfo.intersectionSize == 0) continue;
					if (!nextToPlace
						|| (relativeInfoNextToPlace.intersectionSize != relativeInfo.intersectionSize
								? relativeInfoNextToPlace.intersectionSize < relativeInfo.intersectionSize
								: relativeInfoNextToPlace.commentDiffCount > relativeInfo.commentDiffCount))
					{
						nextToPlace = &connectedGraph;
						relativeInfoNextToPlace = relativeInfo;
					}
				}
				if (nextToPlace) toPlaceConnectedGraph = nextToPlace;
				if (toPlaceConnectedGraph->isPlaced)
				{
					toPlaceConnectedGraphStack
						.pop_back(); // remove last only if already placed and no new connected graph to place related to current
					continue;
				}
				toPlaceConnectedGraphStack.push_back(toPlaceConnectedGraph);

				OldConnectedGraph::CommentRelativeInfo relativeInfoToPlace
					= OldConnectedGraph::CommentRelativeInfo(*toPlaceConnectedGraph, *lastConnectedGraphPlaced);
				const Box* currentBox = toPlaceConnectedGraph->getConnectedGraphBox();

				Vector2 toAdd;
				if (graphConfig.GetIsAI_Graph())
				{
					Box extendedCurrentBox = currentBox->computeBoxToNotOverlapInY(relativeInfoToPlace.commentHeaderCount,
						relativeInfoToPlace.commentDiffCount,
						graphConfig.spacing,
						graphConfig.commentSpacing);
					double toAddInX
						= lastBox->Max.x - extendedCurrentBox.Min.x + 2 * graphConfig.commentSpacing.x + graphConfig.spacing.x;
					double toAddInY = (connectedGraphList[0].connectedCommentSet.empty()
											  ? connectedGraphList[0].globalRootNode->pos.y
											  : connectedGraphList[0].getCommentConnectedGraphBox()->Min.y)
									  - (toPlaceConnectedGraph->connectedCommentSet.empty()
											  ? toPlaceConnectedGraph->globalRootNode->pos.y
											  : toPlaceConnectedGraph->getCommentConnectedGraphBox()->Min.y);
					toAdd = Vector2(toAddInX, toAddInY);
				}
				else
				{
					Box extendedCurrentBox = currentBox->computeBoxToNotOverlapInX(relativeInfoToPlace.commentHeaderCount,
						relativeInfoToPlace.commentDiffCount,
						graphConfig.spacing,
						graphConfig.commentSpacing);
					double toAddInX = (connectedGraphList[0].connectedCommentSet.empty()
											  ? connectedGraphList[0].globalRootNode->pos.x
											  : connectedGraphList[0].getCommentConnectedGraphBox()->Min.x)
									  - (toPlaceConnectedGraph->connectedCommentSet.empty()
											  ? toPlaceConnectedGraph->globalRootNode->pos.x
											  : toPlaceConnectedGraph->getCommentConnectedGraphBox()->Min.x);
					double toAddInY
						= lastBox->Max.y - extendedCurrentBox.Min.y + 2 * graphConfig.commentSpacing.y + graphConfig.spacing.y;
					toAdd = Vector2(toAddInX, toAddInY);
				}
				toPlaceConnectedGraph->moveConnectedGraph(toAdd);
				lastBox = toPlaceConnectedGraph->getConnectedGraphBox();
				lastConnectedGraphPlaced = toPlaceConnectedGraph;
				lastConnectedGraphPlaced->isPlaced = true;
			}
		}

		instructionGlobalContainer.updateMergedNodeInstructionList();
	}
};