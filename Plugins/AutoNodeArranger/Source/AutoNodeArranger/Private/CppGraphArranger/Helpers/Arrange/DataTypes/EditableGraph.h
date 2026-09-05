// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "../../../DataTypes/Highlight.h"
#include "../../../DataTypes/Instruction/GraphInstruction.h"
#include "./ExtraGraph.h"
#include "./ExtraLink.h"
#include "./ExtraNode.h"

#include <chrono>

#define CREATE_REROUTE(category, linkId, bBreakCreatedLinks) editableGraph.CreateReroute(#category, linkId, bBreakCreatedLinks)
#define DELETE_NODE(category, nodeId)						 editableGraph.DeleteNode(#category, nodeId)
#define TEMP_DELETE_REROUTE(category, nodeId)				 editableGraph.TempDeleteReroute(#category, nodeId)
#define RESTORE_REROUTE(category, leftPinId, rightPinId, rerouteNodeId)                                                          \
	editableGraph.RestoreReroute(#category, leftPinId, rightPinId, rerouteNodeId)
#define CHANGE_LINK_LEFT_PIN(category, linkId, newLeftPinId) editableGraph.ChangeLinkLeftPin(#category, linkId, newLeftPinId)
#define MOVE_NODE(category, nodeId, toPos)					 editableGraph.MoveNode(#category, nodeId, toPos)

struct EditableGraph
{
	// ======== ATTRIBUTES ========

	CppGraph graph;
	ExtraGraph extraGraph;

private:
	GraphInstruction graphInstruction;

	// key: rerouteNodeId, value: position
	std::map<size_t, Vector2> tempDeleteRerouteMap;

	std::chrono::time_point<std::chrono::steady_clock> timeout;

public:
	const GraphInstruction& GetGraphInstruction() const { return graphInstruction; }

	// ======== CONSTRUCTOR ========

	EditableGraph() = default;
	explicit EditableGraph(const CppGraph& graph_) : graph(graph_), extraGraph(graph_, true) {}

	// ======== METHODS ========

	void InitializeTimeout() { timeout = std::chrono::steady_clock::now() + std::chrono::seconds(3); }

	void CheckTimeout() const
	{
		if (std::chrono::steady_clock::now() > timeout) throw std::runtime_error("Timeout");
	}

	// must be called after LoopLinkHandler::BreakAllLoops and RerouteCleaner::CleanReroutes
	void SortLink()
	{
		// sort the links of all nodes according to the position of the pins
		for (auto& [nodeId, extraNode] : extraGraph.extraNodeList)
		{
			for (auto linkIdList : extraNode.GetLinkIdListList({ELinkFlag::All}))
			{
				if (linkIdList->empty()) continue;
				bool isLeftLink = extraGraph.extraLinkList.at(linkIdList->front()).GetNode(graph, true).id == nodeId;
				linkIdList->sort(
					[&](size_t lhsLinkId, size_t rhsLinkId)
					{
						auto& lhsExtraLink = extraGraph.extraLinkList.at(lhsLinkId);
						auto& rhsExtraLink = extraGraph.extraLinkList.at(rhsLinkId);

						auto lhsOwnerPinOffset = lhsExtraLink.GetPin(graph, isLeftLink).offset;
						auto rhsOwnerPinOffset = rhsExtraLink.GetPin(graph, isLeftLink).offset;
						if (lhsOwnerPinOffset != rhsOwnerPinOffset) return lhsOwnerPinOffset < rhsOwnerPinOffset;

						auto lhsTargetPinOffset = lhsExtraLink.GetPin(graph, !isLeftLink).offset;
						auto rhsTargetPinOffset = rhsExtraLink.GetPin(graph, !isLeftLink).offset;
						return lhsTargetPinOffset < rhsTargetPinOffset;
					});
			}
		}
	}

	// must be called after LoopLinkHandler::BreakAllLoops and RerouteCleaner::CleanReroutes
	void UpdateAllCommentInfo()
	{
		// update comments
		for (auto& [_, extraComment] : extraGraph.extraCommentList)
		{
			auto& insideNodeIdSet = extraComment.insideNodeIdSet;
			auto& leftRightNodeIdSet = extraComment.leftRightNodeIdSet;
			auto& leftEdgeOutsideFirstRightNodeIdSet = extraComment.leftEdgeOutsideFirstRightNodeIdSet;
			auto& rightEdgeOutsideFirstRightNodeIdSet = extraComment.rightEdgeOutsideFirstRightNodeIdSet;
			std::set<size_t> leftNodeIdSet;
			std::set<size_t> rightNodeIdSet;
			std::set<size_t> rightEdgeOutsideNodeIdSet;
			std::set<size_t> leftEdgeOutsideNodeIdSet;
			for (size_t insideNodeId : insideNodeIdSet)
			{
				// insideNodeId removed later from leftNodeIdSet
				updateNodeIdSet(insideNodeId, leftNodeIdSet, ELinkFlag::Left, -1); // -1 for infinite recursion
				// insideNodeId removed later from rightNodeIdSet
				updateNodeIdSet(insideNodeId, rightNodeIdSet, ELinkFlag::Right, -1); // -1 for infinite recursion
				// insideNodeId and leftRight nodes removed later from leftEdgeOutsideFirstRightNodeIdSet
				updateNodeIdSet(insideNodeId, leftEdgeOutsideFirstRightNodeIdSet, ELinkFlag::Left, 1);
				// insideNodeId and leftRight nodes removed later from rightEdgeOutsideNodeIdSet
				updateNodeIdSet(insideNodeId, rightEdgeOutsideNodeIdSet, ELinkFlag::Right, 1);
				// insideNodeId and leftRight nodes removed later from leftEdgeOutsideNodeIdSet
				updateNodeIdSet(insideNodeId, leftEdgeOutsideNodeIdSet, ELinkFlag::Left, 1);

				auto firstRightLinkId = extraGraph.GetFirstLinkId(insideNodeId, {ELinkFlag::RightExec, ELinkFlag::RightNonExec});
				if (firstRightLinkId != static_cast<size_t>(-1))
				{
					auto& firstRightExtraLink = extraGraph.extraLinkList.at(firstRightLinkId);
					// insideNodeId and leftRight nodes removed later from rightEdgeOutsideNodeIdSet
					rightEdgeOutsideFirstRightNodeIdSet.insert(firstRightExtraLink.GetNode(graph, false).id);
				}
			}

			leftNodeIdSet = difference(leftNodeIdSet, insideNodeIdSet);
			rightNodeIdSet = difference(rightNodeIdSet, insideNodeIdSet);
			leftRightNodeIdSet = intersection(leftNodeIdSet, rightNodeIdSet);
			leftNodeIdSet = difference(leftNodeIdSet, leftRightNodeIdSet);
			rightNodeIdSet = difference(rightNodeIdSet, leftRightNodeIdSet);
			rightEdgeOutsideFirstRightNodeIdSet = difference(rightEdgeOutsideFirstRightNodeIdSet, insideNodeIdSet);
			rightEdgeOutsideFirstRightNodeIdSet = difference(rightEdgeOutsideFirstRightNodeIdSet, leftRightNodeIdSet);

			if (graph.graphConfig.GetIsAI_Graph()) // force inclusion of leftRight nodes for AI graph
			{
				insideNodeIdSet.insert(leftRightNodeIdSet.begin(), leftRightNodeIdSet.end());

				// update nodes
				for (size_t leftRightNodeId : leftRightNodeIdSet)
					extraGraph.extraNodeList.at(leftRightNodeId).insideCommentIdSet.insert(extraComment.id);

				leftRightNodeIdSet.clear();
			}

			leftEdgeOutsideFirstRightNodeIdSet = difference(leftEdgeOutsideFirstRightNodeIdSet, insideNodeIdSet);
			leftEdgeOutsideFirstRightNodeIdSet = difference(leftEdgeOutsideFirstRightNodeIdSet, leftRightNodeIdSet);
			rightEdgeOutsideNodeIdSet = difference(rightEdgeOutsideNodeIdSet, insideNodeIdSet);
			rightEdgeOutsideNodeIdSet = difference(rightEdgeOutsideNodeIdSet, leftRightNodeIdSet);
			leftEdgeOutsideNodeIdSet = difference(leftEdgeOutsideNodeIdSet, insideNodeIdSet);
			leftEdgeOutsideNodeIdSet = difference(leftEdgeOutsideNodeIdSet, leftRightNodeIdSet);

			// update nodes
			for (size_t leftRightNodeId : leftRightNodeIdSet)
				extraGraph.extraNodeList.at(leftRightNodeId).leftRightCommentIdSet.insert(extraComment.id);

			// careful: right for node means left for comment
			for (size_t rightEdgeOutsideNodeId : rightEdgeOutsideNodeIdSet)
				extraGraph.extraNodeList.at(rightEdgeOutsideNodeId).leftEdgeOutsideCommentIdSet.insert(extraComment.id);
			// careful: left for node means right for comment
			for (size_t leftEdgeOutsideNodeId : leftEdgeOutsideNodeIdSet)
				extraGraph.extraNodeList.at(leftEdgeOutsideNodeId).rightEdgeOutsideCommentIdSet.insert(extraComment.id);

			size_t extraCommentId = extraComment.id;
			auto bEnterInCommentOnFirstRight = [&, extraCommentId](size_t nodeId)
			{
				auto firstRightLinkId = extraGraph.GetFirstLinkId(nodeId, {ELinkFlag::RightExec, ELinkFlag::RightNonExec});
				if (firstRightLinkId == static_cast<size_t>(-1)) return false;
				auto& firstRightExtraLink = extraGraph.extraLinkList.at(firstRightLinkId);
				auto& firstRightExtraNode = extraGraph.extraNodeList.at(firstRightExtraLink.GetNode(graph, false).id);
				return contains(firstRightExtraNode.insideCommentIdSet, extraCommentId);
			};
			leftEdgeOutsideFirstRightNodeIdSet
				= copy_if_set<size_t>(leftEdgeOutsideFirstRightNodeIdSet, bEnterInCommentOnFirstRight);
		}

		for (auto& [_, extraNode] : extraGraph.extraNodeList)
		{
			auto& insideCommentIdSet = extraNode.insideCommentIdSet;
			auto& leftRightCommentIdSet = extraNode.leftRightCommentIdSet;
			for (size_t insideCommentId : insideCommentIdSet)
			{
				auto& extraComment = extraGraph.extraCommentList[insideCommentId];
				extraComment.leftRightCommentIdSet.insert(leftRightCommentIdSet.begin(), leftRightCommentIdSet.end());
				for (size_t leftRightCommentId : leftRightCommentIdSet)
				{
					auto& leftRightExtraComment = extraGraph.extraCommentList[leftRightCommentId];
					leftRightExtraComment.leftRightCommentIdSet.insert(insideCommentId);
				}
				for (size_t neighbourNodeId : extraComment.insideNodeIdSet)
				{
					auto& neighbourExtraNode = extraGraph.extraNodeList.at(neighbourNodeId);
					neighbourExtraNode.neighbourLeftRightCommentIdSet.insert(
						leftRightCommentIdSet.begin(), leftRightCommentIdSet.end());
				}
			}
		}
	}

	void CreateReroute(const std::string& category, int linkId, bool bBreakCreatedLinks)
	{
		auto& extraLink = extraGraph.extraLinkList.at(linkId);
		extraLink.isBroken = true; // better to break link instead of remove from list
		bool isExec = extraLink.isExec;
		bool bFinishWithRerouteOneOutput = extraLink.bFinishWithRerouteOneOutput;

		// add reroute node one output
		size_t rerouteNodeId = graph.nodeList.size();
		Vector2 rerouteNodePos = (extraLink.GetPin(graph, true).offset + extraLink.GetNode(graph, true).box.Min
									 + extraLink.GetPin(graph, false).offset + extraLink.GetNode(graph, false).box.Min)
								 / 2;
		graph.nodeList[rerouteNodeId]
			= CppNode(rerouteNodeId, "Reroute" + std::to_string(rerouteNodeId), Box(rerouteNodePos, rerouteNodePos), true);

		// add extraNode
		size_t newNodeId = extraGraph.extraNodeList.size();
		ExtraNode extraNode;
		extraNode.isOneOutputRerouteNode = true;
		extraGraph.extraNodeList[newNodeId] = extraNode;

		// add pins
		size_t rerouteNodeInputPinId = graph.pinList.size();
		auto inputPinName = "Input" + std::to_string(rerouteNodeInputPinId);
		graph.pinList[rerouteNodeInputPinId]
			= CppPin(rerouteNodeInputPinId, rerouteNodeId, inputPinName, isExec, true, REROUTE_PIN_OFFSET);
		size_t rerouteNodeOutputPinId = graph.pinList.size();
		auto outputPinName = "Output" + std::to_string(rerouteNodeOutputPinId);
		graph.pinList[rerouteNodeOutputPinId]
			= CppPin(rerouteNodeOutputPinId, rerouteNodeId, outputPinName, isExec, false, REROUTE_PIN_OFFSET);

		auto& rerouteNode = extraGraph.extraNodeList.at(rerouteNodeId);
		rerouteNode.GetLinkIdList(true, isExec).push_back(linkId);

		auto& link = graph.linkList.at(linkId);
		// connect link.leftPinId to rerouteNodeInputPinId
		auto& leftNode = graph.nodeList.at(extraLink.GetNode(graph, true).id);
		size_t newLeftLinkId
			= addLink(link.leftPinId, rerouteNodeInputPinId, isExec, true, leftNode.isRerouteNode, bBreakCreatedLinks);
		// connect rerouteNodeOutputPinId to link.rightPinId
		size_t newRightLinkId = addLink(rerouteNodeOutputPinId,
			link.rightPinId,
			isExec,
			bFinishWithRerouteOneOutput,
			bFinishWithRerouteOneOutput,
			bBreakCreatedLinks);

		// add instruction

		// create node
		graphInstruction.rerouteInstructionList.emplace_back(rerouteNodePos);
		HIGHLIGHT(category, graphInstruction.rerouteInstructionList.back(), rerouteNodeId);
		// break old link
		graphInstruction.linkInstructionList.emplace_back(link.leftPinId, link.rightPinId, true);
		HIGHLIGHT(category, graphInstruction.linkInstructionList.back());
		// connect left pin to reroute node
		graphInstruction.linkInstructionList.emplace_back(link.leftPinId, rerouteNodeInputPinId, false);
		HIGHLIGHT(category, graphInstruction.linkInstructionList.back(), newLeftLinkId);
		// connect reroute node to right pin
		graphInstruction.linkInstructionList.emplace_back(rerouteNodeOutputPinId, link.rightPinId, false);
		HIGHLIGHT(category, graphInstruction.linkInstructionList.back(), newRightLinkId);
	}

	void CopyNodeComments(size_t fromNodeId, size_t toNodeId)
	{
		auto& fromExtraNode = extraGraph.extraNodeList.at(fromNodeId);
		auto& toExtraNode = extraGraph.extraNodeList.at(toNodeId);
		for (size_t commentId : fromExtraNode.insideCommentIdSet)
		{
			extraGraph.extraCommentList.at(commentId).insideNodeIdSet.insert(toNodeId);
			toExtraNode.insideCommentIdSet.insert(commentId);
		}
	}

	void DeleteNode(const std::string& category, size_t nodeId)
	{
		// delete node
		auto& extraNode = extraGraph.extraNodeList.at(nodeId);
		extraNode.isDeleted = true;
		// break all links
		for (const auto& linkIdList : extraNode.GetLinkIdListList({ELinkFlag::All}))
			for (auto& linkId : *linkIdList)
			{
				auto& link = graph.linkList.at(linkId);
				extraGraph.extraLinkList.at(linkId).isBroken = true;
				// add instruction
				graphInstruction.linkInstructionList.emplace_back(link.leftPinId, link.rightPinId, true);
				HIGHLIGHT(category, graphInstruction.linkInstructionList.back(), linkId);
			}
		// add instruction
		graphInstruction.deleteInstructionList.emplace_back(nodeId, false);
		HIGHLIGHT(category, graphInstruction.deleteInstructionList.back(), nodeId);
	}

	void TempDeleteReroute(const std::string& category, size_t nodeId)
	{
		// delete node
		auto& extraNode = extraGraph.extraNodeList.at(nodeId);
		extraNode.isDeleted = true;
		bool isLeftRerouteNode = false;
		size_t firstLeftLinkId = extraGraph.GetFirstLinkId(nodeId, {ELinkFlag::Left});
		size_t firstLeftPinId = static_cast<size_t>(-1);
		size_t leftNodeId = static_cast<size_t>(-1);
		if (firstLeftLinkId != static_cast<size_t>(-1))
		{
			auto& link = graph.linkList.at(firstLeftLinkId);
			firstLeftPinId = link.leftPinId;
			leftNodeId = graph.pinList.at(firstLeftPinId).ownerNodeId;
			isLeftRerouteNode = graph.nodeList.at(leftNodeId).isRerouteNode;
		}

		// break all links and create new links with origin of the reroute node
		for (const auto& linkIdList : extraNode.GetLinkIdListList({ELinkFlag::All}))
			for (auto& linkId : *linkIdList)
			{
				CheckTimeout();
				auto& extraLink = extraGraph.extraLinkList.at(linkId);
				if (extraLink.isBroken) continue;
				extraLink.isBroken = true;
				auto& link = graph.linkList.at(linkId);
				auto& leftNode = extraLink.GetNode(graph, true);
				// add instruction
				graphInstruction.linkInstructionList.emplace_back(link.leftPinId, link.rightPinId, true);
				HIGHLIGHT(category, graphInstruction.linkInstructionList.back(), linkId);
				if (leftNodeId == leftNode.id) continue; // do not create new link if left node is the origin of the reroute node
				// create new link
				size_t newLinkId = addLink(firstLeftPinId,
					link.rightPinId,
					extraLink.isExec,
					extraLink.bFinishWithRerouteOneOutput,
					isLeftRerouteNode,
					false);
				graphInstruction.linkInstructionList.emplace_back(firstLeftPinId, link.rightPinId, false);
				HIGHLIGHT(category, graphInstruction.linkInstructionList.back(), newLinkId);
				// store deleted reroute and link
				tempDeleteRerouteMap[nodeId] = graph.nodeList.at(nodeId).box.Min;
			}
		// add instruction
		graphInstruction.deleteInstructionList.emplace_back(nodeId, false);
		HIGHLIGHT(category, graphInstruction.deleteInstructionList.back(), nodeId);
	}

	size_t ChangeLinkLeftPin(const std::string& category, size_t linkId, size_t newLeftPinId)
	{
		auto& link = graph.linkList.at(linkId);
		auto& extraLink = extraGraph.extraLinkList.at(linkId);

		// break old link
		extraLink.isBroken = true;
		graphInstruction.linkInstructionList.emplace_back(link.leftPinId, link.rightPinId, true);
		HIGHLIGHT(category, graphInstruction.linkInstructionList.back(), linkId);

		// create new link
		auto& leftNode = extraLink.GetNode(graph, false);
		auto& rightNode = extraLink.GetNode(graph, true);
		size_t newLinkId = addLink(newLeftPinId,
			link.rightPinId,
			extraLink.isExec,
			rightNode.isRerouteNode,
			leftNode.isRerouteNode && rightNode.isRerouteNode,
			false);
		graphInstruction.linkInstructionList.emplace_back(newLeftPinId, link.rightPinId, false);
		HIGHLIGHT(category, graphInstruction.linkInstructionList.back(), newLinkId);

		return newLinkId;
	}

	void MoveNode(const std::string& category, size_t nodeId, Vector2 toPos)
	{
		// move node
		auto& node = graph.nodeList.at(nodeId);
		Vector2 fromPos = node.box.Min;
		if (fromPos == toPos) return;
		node.box = node.box.offsetBox(toPos - node.box.Min);
		// add instruction
		graphInstruction.allNodeInstructionList.emplace_back(nodeId, fromPos, toPos);
		HIGHLIGHT(category, graphInstruction.allNodeInstructionList.back(), nodeId);
	}

	void SetAllCommentInstructions()
	{
		for (const auto& [_, extraComment] : extraGraph.extraCommentList)
		{
			graphInstruction.commentInstructionList.emplace_back(extraComment.id);
			auto& commentInstruction = graphInstruction.commentInstructionList.back();
			for (size_t nodeId : extraComment.insideNodeIdSet) commentInstruction.nodeIdUnderCommentList.push_back(nodeId);
			for (size_t commentId : extraComment.insideCommentIdSet)
				commentInstruction.commentIdUnderCommentList.push_back(commentId);
		}
	}

	void ReconciliateRerouteInstructions()
	{
		size_t rerouteNodeId = graph.nodeList.size() - graphInstruction.rerouteInstructionList.size();
		for (auto& rerouteInstruction : graphInstruction.rerouteInstructionList)
		{
			if (tempDeleteRerouteMap.empty()) break;
			// find closest deleted reroute node
			auto bestIt = tempDeleteRerouteMap.begin();
			auto& rerouteNodeBox = graph.nodeList.at(rerouteNodeId).box;
			double minSquareDistance = (bestIt->second - rerouteNodeBox.Min).squareLength();
			for (auto it = tempDeleteRerouteMap.begin(); it != tempDeleteRerouteMap.end(); ++it)
			{
				double squareDistance = (it->second - rerouteNodeBox.Min).squareLength();
				if (squareDistance < minSquareDistance)
				{
					minSquareDistance = squareDistance;
					bestIt = it;
				}
			}
			rerouteInstruction.pos = bestIt->second;
			tempDeleteRerouteMap.erase(bestIt);
			// update the 'from' of the first move instruction of the created reroute node
			for (auto& nodeInstruction : graphInstruction.allNodeInstructionList)
				if (nodeInstruction.nodeId == rerouteNodeId)
				{
					nodeInstruction.fromPos = rerouteInstruction.pos;
					break;
				}

			rerouteNodeId++;
		}
		// highlights not reconciliated
	}

	void SetMergedNodeInstructionList() { graphInstruction.updateMergedNodeInstructionList(); }

private:
	// be sure to use a signed type for recursiveCount
	void updateNodeIdSet(size_t nodeId, std::set<size_t>& nodeIdSet, ELinkFlag linkFlag, int32_t recursiveCount)
	{
		if (recursiveCount < 0 && contains(nodeIdSet, nodeId)) return;
		nodeIdSet.insert(nodeId);
		if (recursiveCount == 0) return;
		for (auto linkIdList : extraGraph.extraNodeList.at(nodeId).GetLinkIdListList({linkFlag}))
			for (size_t linkId : *linkIdList)
			{
				auto& extraLink = extraGraph.extraLinkList.at(linkId);
				if (extraLink.isBroken) continue;
				auto& otherNode = extraLink.GetNodeConnectedTo(graph, nodeId);
				updateNodeIdSet(otherNode.id, nodeIdSet, linkFlag, recursiveCount - 1);
			}
	}

	size_t addLink(size_t leftPinId,
		size_t rightPinId,
		bool isExec,
		bool bFinishWithRerouteOneOutput,
		bool bBetweenTwoRerouteOneOutput,
		bool isBroken)
	{
		auto& leftNodeId = graph.pinList.at(leftPinId).ownerNodeId;
		auto& rightNodeId = graph.pinList.at(rightPinId).ownerNodeId;
		size_t newLinkId = graph.linkList.size();
		graph.linkList[newLinkId] = (CppLink(newLinkId, leftPinId, rightPinId));
		// add link to extraNodes
		extraGraph.extraNodeList.at(leftNodeId).GetLinkIdList(false, isExec).push_back(newLinkId);
		extraGraph.extraNodeList.at(rightNodeId).GetLinkIdList(true, isExec).push_back(newLinkId);
		// add extraLink
		ExtraLink newExtraLink;
		newExtraLink.id = newLinkId;
		newExtraLink.isExec = isExec;
		newExtraLink.nodeDiffX = newExtraLink.GetNode(graph, false).box.Min.x - newExtraLink.GetNode(graph, true).box.Min.x;
		newExtraLink.pinDiffY = newExtraLink.GetPin(graph, false).offset.y - newExtraLink.GetPin(graph, true).offset.y;
		newExtraLink.bFinishWithRerouteOneOutput = bFinishWithRerouteOneOutput;
		newExtraLink.bBetweenTwoRerouteOneOutput = bBetweenTwoRerouteOneOutput;
		newExtraLink.isBroken = isBroken;
		extraGraph.extraLinkList[newLinkId] = newExtraLink;

		return newLinkId;
	}
};
