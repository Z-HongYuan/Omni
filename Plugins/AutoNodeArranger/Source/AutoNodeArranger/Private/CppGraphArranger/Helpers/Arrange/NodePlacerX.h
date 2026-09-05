// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <map>
#include <set>
#include <vector>

#include "../../DataTypes/Highlight.h"
#include "./NodePlacerCommon.h"

#define PLACE_NODE_IN_MIN_X
#define PLACE_NODE_IN_MAX_X
#define PLACE_LOOP_CHILD
#define PLACE_POS_MAP
#define PLACE_DIFF_MAP

class NodePlacerX
{
public:
	static void PlaceGraphInX(size_t root, EditableGraph& editableGraph)
	{
		std::set<size_t> placedNodeIdSet;
		placedNodeIdSet.insert(root);

		std::set<size_t> newlyPlacedNodeIdSet = placedNodeIdSet;

		while (!newlyPlacedNodeIdSet.empty())
		{
			std::set<size_t> startPlacedNodeIdSet = placedNodeIdSet;

			std::set<size_t> visitedNodeIdSet;
			// map used to place nodes on the opposite placing direction, key is node id to place, value is pos
			std::map<size_t, double> posMap;
			// map used to limit the pos on the opposite placing direction, key is node id to place, value is pos
			std::map<size_t, double> limitPosMap;
			// map used to compute posMap, key is node id to place, value is map of node id to reference, value is diff
			std::map<size_t, std::map<size_t, double>> diffMapMap;

			for (size_t placedNodeId : startPlacedNodeIdSet)
				placeNodeInMinX(
					placedNodeId, visitedNodeIdSet, placedNodeIdSet, posMap, limitPosMap, diffMapMap, editableGraph, true);

			placePosMap(posMap, diffMapMap, editableGraph);
			placePosMap(limitPosMap, diffMapMap, editableGraph);

			std::set<size_t> diffPlacedNodeIdSet;
			for (auto& diffMap : diffMapMap)
				placeDiffMap(diffMap.first, diffPlacedNodeIdSet, posMap, limitPosMap, diffMapMap, editableGraph, true);

			visitedNodeIdSet.clear();
			// placedNodeIdSet intentionally not cleared to avoid placing nodes that are already placed
			posMap.clear();
			limitPosMap.clear();
			diffMapMap.clear();
			for (size_t placedNodeId : difference(placedNodeIdSet, startPlacedNodeIdSet))
				placeNodeInMaxX(
					placedNodeId, visitedNodeIdSet, placedNodeIdSet, posMap, limitPosMap, diffMapMap, editableGraph, true);

			placePosMap(posMap, diffMapMap, editableGraph);
			placePosMap(limitPosMap, diffMapMap, editableGraph);

			diffPlacedNodeIdSet.clear();
			for (auto& diffMap : diffMapMap)
				placeDiffMap(diffMap.first, diffPlacedNodeIdSet, posMap, limitPosMap, diffMapMap, editableGraph, false);

			newlyPlacedNodeIdSet = difference(placedNodeIdSet, startPlacedNodeIdSet);
		}
	}

private:
	struct Diff
	{
		Diff(size_t nodeId, double diff) : nodeId(nodeId), diff(diff) {}

		size_t nodeId;
		double diff;
	};

	static double getCommentDiffSpacingX(
		const ExtraNode& extraNode, const ExtraNode& leftExtraNode, const CppGraphConfig& graphConfig)
	{
		bool isLeftRightComment = !intersection(extraNode.leftRightCommentIdSet, leftExtraNode.insideCommentIdSet).empty()
								  || !intersection(leftExtraNode.leftRightCommentIdSet, extraNode.insideCommentIdSet).empty();
		bool isNeighbourLeftRightComment
			= !intersection(extraNode.neighbourLeftRightCommentIdSet, leftExtraNode.insideCommentIdSet).empty()
			  || !intersection(leftExtraNode.neighbourLeftRightCommentIdSet, extraNode.insideCommentIdSet).empty();
		auto extraToLeftDiffInside = difference(extraNode.insideCommentIdSet, leftExtraNode.insideCommentIdSet);
		auto leftToExtraDiffInside = difference(leftExtraNode.insideCommentIdSet, extraNode.insideCommentIdSet);
		size_t baseCommentDiffCount = extraToLeftDiffInside.size() + leftToExtraDiffInside.size();
		size_t commentDiffCount = isLeftRightComment ? 0 : isNeighbourLeftRightComment ? 1 : baseCommentDiffCount;
		return graphConfig.commentSpacing.x * commentDiffCount;
	}

	static void placeNodeInMinX(size_t nodeId,
		std::set<size_t>& visitedNodeIdSet,
		std::set<size_t>& placedNodeIdSet,
		std::map<size_t, double>& posMap,
		std::map<size_t, double>& limitPosMap,
		std::map<size_t, std::map<size_t, double>>& diffMapMap,
		EditableGraph& editableGraph,
		bool bRecursive)
	{
		editableGraph.CheckTimeout();
		if (!bRecursive && contains(placedNodeIdSet, nodeId)) return; // non recursive call is only for placing nodes
		if (contains(visitedNodeIdSet, nodeId)) return;
		visitedNodeIdSet.insert(nodeId);

		const auto& extraGraph = editableGraph.extraGraph;
		const auto& graph = editableGraph.graph;
		const auto& graphConfig = graph.graphConfig;

		auto& node = graph.nodeList.at(nodeId);
		auto& extraNode = extraGraph.extraNodeList.at(nodeId);

		double minPosX = contains(placedNodeIdSet, nodeId) ? node.box.Min.x : INVALID_MIN_POS;

		for (const auto& leftLinkIdList : extraNode.GetLinkIdListList({ELinkFlag::Left}))
			for (const auto& leftLinkId : *leftLinkIdList)
			{
				auto& leftExtraLink = extraGraph.extraLinkList.at(leftLinkId);
				if (leftExtraLink.isBroken) continue;
				auto& leftNode = leftExtraLink.GetNodeConnectedTo(graph, nodeId);
				auto& leftExtraNode = extraGraph.extraNodeList.at(leftNode.id);
				// place left node
				placeNodeInMinX(
					leftNode.id, visitedNodeIdSet, placedNodeIdSet, posMap, limitPosMap, diffMapMap, editableGraph, false);
				if (contains(placedNodeIdSet, leftNode.id))
				{
					// consider only placed nodes
					double linkSpacingX = leftExtraLink.isExec ? graphConfig.execSpacingX : graphConfig.spacing.x;
					size_t commentDiffSpacingX = getCommentDiffSpacingX(extraNode, leftExtraNode, graphConfig);
					minPosX = std::max(minPosX, leftNode.box.Max.x + linkSpacingX + commentDiffSpacingX);
					updateDiffMapMap(
						diffMapMap, leftNode.id, Diff(nodeId, leftNode.box.getSize().x + linkSpacingX + commentDiffSpacingX));
				}
			}

		// place nodes that enter in comments containing current node to the first right
		for (size_t insideCommentId : extraNode.insideCommentIdSet)
		{
			auto& insideExtraComment = extraGraph.extraCommentList.at(insideCommentId);
			double maxEdgeOutsidePosX = INVALID_MIN_POS; // careful, width included
			for (size_t leftEdgeOutsideFirstRightNodeId : insideExtraComment.leftEdgeOutsideFirstRightNodeIdSet)
			{
				placeNodeInMinX(leftEdgeOutsideFirstRightNodeId,
					visitedNodeIdSet,
					placedNodeIdSet,
					posMap,
					limitPosMap,
					diffMapMap,
					editableGraph,
					false);
				if (contains(placedNodeIdSet, leftEdgeOutsideFirstRightNodeId))
				{
					// consider only placed nodes
					auto leftOutsideNode = graph.nodeList.at(leftEdgeOutsideFirstRightNodeId);
					auto leftExtraNode = extraGraph.extraNodeList.at(leftEdgeOutsideFirstRightNodeId);
					double commentDiffSpacingX = getCommentDiffSpacingX(extraNode, leftExtraNode, graphConfig);
					minPosX = std::max(minPosX, leftOutsideNode.box.Max.x + graphConfig.commentSpacing.x + commentDiffSpacingX);
					maxEdgeOutsidePosX = std::max(maxEdgeOutsidePosX, leftOutsideNode.box.Max.x);
				}
			}
			if (maxEdgeOutsidePosX != INVALID_MIN_POS)
				for (size_t leftEdgeOutsideFirstRightNodeId : insideExtraComment.leftEdgeOutsideFirstRightNodeIdSet)
				{
					auto leftOutsideNode = graph.nodeList.at(leftEdgeOutsideFirstRightNodeId);
					updatePosMap(posMap, leftOutsideNode.id, maxEdgeOutsidePosX - leftOutsideNode.box.getSize().x, true);
				}
		}

		// place comment to the left of current node
		for (const auto& leftEdgeOutsideCommentId : extraNode.leftEdgeOutsideCommentIdSet)
		{
			auto& extraComment = extraGraph.extraCommentList.at(leftEdgeOutsideCommentId);
			double maxEdgeOutsidePosX = INVALID_MIN_POS;
			for (size_t insideNodeId : extraComment.insideNodeIdSet)
			{
				placeNodeInMinX(
					insideNodeId, visitedNodeIdSet, placedNodeIdSet, posMap, limitPosMap, diffMapMap, editableGraph, false);
				if (contains(placedNodeIdSet, insideNodeId))
				{
					// consider only placed nodes
					auto insideNode = graph.nodeList.at(insideNodeId);
					auto insideExtraNode = extraGraph.extraNodeList.at(insideNodeId);
					double commentDiffSpacingX = getCommentDiffSpacingX(extraNode, insideExtraNode, graphConfig);
					minPosX = std::max(minPosX, insideNode.box.Max.x + graphConfig.commentSpacing.x + commentDiffSpacingX);
					maxEdgeOutsidePosX = std::max(maxEdgeOutsidePosX, insideNode.box.Max.x);
				}
			}
			if (maxEdgeOutsidePosX != INVALID_MIN_POS)
				for (size_t insideNodeId : extraComment.insideNodeIdSet)
				{
					auto insideNode = graph.nodeList.at(insideNodeId);
					updatePosMap(limitPosMap, insideNode.id, maxEdgeOutsidePosX - insideNode.box.getSize().x, false);
				}
		}

		if (minPosX != INVALID_MIN_POS)
		{
			// place node
			MOVE_NODE(PLACE_NODE_IN_MIN_X, nodeId, Vector2(minPosX, node.box.Min.y));
			placedNodeIdSet.insert(nodeId);

			// place loop children
			for (size_t loopChildId : extraNode.loopChildIdSet)
			{
				visitedNodeIdSet.insert(loopChildId);
				placeLoopChild(loopChildId, nodeId, editableGraph);
			}

			if (bRecursive)
				for (const auto& rightLinkIdList : {extraNode.GetLinkIdList(false, true), extraNode.GetLinkIdList(false, false)})
					for (const auto& rightLinkId : rightLinkIdList)
					{
						auto& rightExtraLink = extraGraph.extraLinkList.at(rightLinkId);
						if (rightExtraLink.isBroken) continue;
						auto& rightNode = rightExtraLink.GetNodeConnectedTo(graph, nodeId);
						// place right node
						placeNodeInMinX(rightNode.id,
							visitedNodeIdSet,
							placedNodeIdSet,
							posMap,
							limitPosMap,
							diffMapMap,
							editableGraph,
							true);
					}
		}
		else if (!bRecursive)
			visitedNodeIdSet.erase(nodeId);
	}

	static void placeLoopChild(size_t childId, size_t parentId, EditableGraph& editableGraph)
	{
		auto& graph = editableGraph.graph;
		auto& extraGraph = editableGraph.extraGraph;
		auto& extraChild = extraGraph.extraNodeList.at(childId);
		auto& parentNode = graph.nodeList.at(parentId);
		auto& childNode = graph.nodeList.at(childId);
		double posX = extraChild.isLoopBrotherPlacedOnRight
						  ? parentNode.box.Max.x - childNode.box.getSize().x - REROUTE_PIN_OFFSET.x
						  : parentNode.box.Min.x + REROUTE_PIN_OFFSET.x;
		double posY = parentNode.box.Max.y + graph.graphConfig.spacing.y;
		MOVE_NODE(PLACE_LOOP_CHILD, childId, Vector2(posX, posY));
	}

	static void updatePosMap(std::map<size_t, double>& posMap, size_t nodeId, double pos, bool bMax)
	{
		auto posMapIt = posMap.find(nodeId);
		if (posMapIt == posMap.end()) posMap[nodeId] = pos;
		else
			posMapIt->second = bMax ? std::max(posMapIt->second, pos) : std::min(posMapIt->second, pos);
	};

	static void updateDiffMapMap(std::map<size_t, std::map<size_t, double>>& diffMapMap, size_t nodeId, const Diff& diff)
	{
		auto diffMapMapIt = diffMapMap.find(nodeId);
		if (diffMapMapIt == diffMapMap.end()) diffMapMap[nodeId] = {{diff.nodeId, diff.diff}};
		else
		{
			auto& diffMap = diffMapMapIt->second;
			auto diffMapIt = diffMap.find(diff.nodeId);
			if (diffMapIt == diffMap.end()) diffMap[diff.nodeId] = diff.diff;
			else
				diffMapIt->second = std::max(diffMapIt->second, diff.diff);
		}
	};

	static void placePosMap(const std::map<size_t, double>& posMap,
		const std::map<size_t, std::map<size_t, double>>& diffMapMap,
		EditableGraph& editableGraph)
	{
		const auto& graph = editableGraph.graph;
		const auto& extraGraph = editableGraph.extraGraph;

		for (auto& [nodeId, pos] : posMap)
		{
			if (diffMapMap.count(nodeId)) continue; // do not place node if it has a diff pos
			auto& node = graph.nodeList.at(nodeId);
			auto& extraNode = extraGraph.extraNodeList.at(nodeId);
			// do not place node connected to the left on exec (that are not a reroute)
			if (!node.isRerouteNode && !extraNode.GetLinkIdList(true, true).empty()) continue;
			MOVE_NODE(PLACE_POS_MAP, nodeId, Vector2(pos, node.box.Min.y));
		}
	}

	static void placeDiffMap(size_t nodeId,
		std::set<size_t>& diffPlacedNodeIdSet,
		const std::map<size_t, double>& posMap,
		const std::map<size_t, double>& limitPosMap,
		const std::map<size_t, std::map<size_t, double>>& diffMapMap,
		EditableGraph& editableGraph,
		bool bMax)
	{
		if (contains(diffPlacedNodeIdSet, nodeId)) return;
		diffPlacedNodeIdSet.insert(nodeId);
		auto diffMapMapIt = diffMapMap.find(nodeId);
		if (diffMapMapIt == diffMapMap.end()) return;
		const auto& graph = editableGraph.graph;
		const auto& extraGraph = editableGraph.extraGraph;
		auto& node = graph.nodeList.at(nodeId);
		auto& extraNode = extraGraph.extraNodeList.at(nodeId);

		// do not place on max node connected to the left on exec (that are not a reroute)
		if (bMax == (!node.isRerouteNode && !extraNode.GetLinkIdList(true, true).empty())) return;

		auto posMapIt = posMap.find(nodeId);
		double pos = posMapIt != posMap.end() ? posMapIt->second : bMax ? -INVALID_MIN_POS : INVALID_MIN_POS;
		for (auto& [refNodeId, diff] : diffMapMapIt->second)
		{
			placeDiffMap(refNodeId, diffPlacedNodeIdSet, posMap, limitPosMap, diffMapMap, editableGraph, bMax);
			auto& refNode = graph.nodeList.at(refNodeId);
			pos = bMax ? std::min(pos, refNode.box.Min.x - diff) : std::max(pos, refNode.box.Min.x - diff);
		}
		auto limitPosMapIt = limitPosMap.find(nodeId);
		if (limitPosMapIt != limitPosMap.end())
			pos = bMax ? std::min(pos, limitPosMapIt->second) : std::max(pos, limitPosMapIt->second);
		MOVE_NODE(PLACE_DIFF_MAP, nodeId, Vector2(pos, node.box.Min.y));
	}

	static void placeNodeInMaxX(size_t nodeId,
		std::set<size_t>& visitedNodeIdSet,
		std::set<size_t>& placedNodeIdSet,
		std::map<size_t, double>& posMap,
		std::map<size_t, double>& limitPosMap,
		std::map<size_t, std::map<size_t, double>>& diffMapMap,
		EditableGraph& editableGraph,
		bool bRecursive)
	{
		editableGraph.CheckTimeout();
		if (!bRecursive && contains(placedNodeIdSet, nodeId)) return; // non recursive call is only for placing nodes
		if (contains(visitedNodeIdSet, nodeId)) return;
		visitedNodeIdSet.insert(nodeId);

		const auto& extraGraph = editableGraph.extraGraph;
		const auto& graph = editableGraph.graph;
		const auto& graphConfig = graph.graphConfig;
		auto& node = graph.nodeList.at(nodeId);
		auto& extraNode = extraGraph.extraNodeList.at(nodeId);

		double maxPosX = contains(placedNodeIdSet, nodeId) ? node.box.Min.x : -INVALID_MIN_POS;

		for (const auto& rightLinkIdList : extraNode.GetLinkIdListList({ELinkFlag::Right}))
			for (const auto& rightLinkId : *rightLinkIdList)
			{
				auto& rightExtraLink = extraGraph.extraLinkList.at(rightLinkId);
				if (rightExtraLink.isBroken) continue;
				auto& rightNode = rightExtraLink.GetNodeConnectedTo(graph, nodeId);
				auto& rightExtraNode = extraGraph.extraNodeList.at(rightNode.id);
				// place right node
				placeNodeInMaxX(
					rightNode.id, visitedNodeIdSet, placedNodeIdSet, posMap, limitPosMap, diffMapMap, editableGraph, false);
				if (contains(placedNodeIdSet, rightNode.id))
				{
					// consider only placed nodes
					double linkSpacingX = rightExtraLink.isExec ? graphConfig.execSpacingX : graphConfig.spacing.x;
					double commentDiffSpacingX = getCommentDiffSpacingX(extraNode, rightExtraNode, graphConfig);
					maxPosX = std::min(maxPosX, rightNode.box.Min.x - linkSpacingX - commentDiffSpacingX - node.box.getSize().x);
					updateDiffMapMap(
						diffMapMap, rightNode.id, Diff(nodeId, -node.box.getSize().x - linkSpacingX - commentDiffSpacingX));
				}
			}

		// place nodes that exit in comments containing current node to the first right
		for (size_t insideCommentId : extraNode.insideCommentIdSet)
		{
			auto& insideExtraComment = extraGraph.extraCommentList.at(insideCommentId);
			double minEdgeOutsidePosX = -INVALID_MIN_POS;
			for (size_t rightEdgeOutsideFirstRightNodeId : insideExtraComment.rightEdgeOutsideFirstRightNodeIdSet)
			{
				placeNodeInMaxX(rightEdgeOutsideFirstRightNodeId,
					visitedNodeIdSet,
					placedNodeIdSet,
					posMap,
					limitPosMap,
					diffMapMap,
					editableGraph,
					false);
				if (contains(placedNodeIdSet, rightEdgeOutsideFirstRightNodeId))
				{
					// consider only placed nodes
					auto rightOutsideNode = graph.nodeList.at(rightEdgeOutsideFirstRightNodeId);
					auto rightExtraNode = extraGraph.extraNodeList.at(rightEdgeOutsideFirstRightNodeId);
					double commentDiffSpacingX = getCommentDiffSpacingX(extraNode, rightExtraNode, graphConfig);
					maxPosX = std::min(maxPosX,
						rightOutsideNode.box.Min.x - graphConfig.commentSpacing.x - commentDiffSpacingX - node.box.getSize().x);
					minEdgeOutsidePosX = std::min(minEdgeOutsidePosX, rightOutsideNode.box.Min.x);
				}
			}
			if (minEdgeOutsidePosX != -INVALID_MIN_POS)
				for (size_t rightEdgeOutsideFirstRightNodeId : insideExtraComment.rightEdgeOutsideFirstRightNodeIdSet)
				{
					auto rightOutsideNode = graph.nodeList.at(rightEdgeOutsideFirstRightNodeId);
					updatePosMap(posMap, rightOutsideNode.id, minEdgeOutsidePosX, false);
				}
		}

		// place comment to the right of current node
		for (const auto& rightEdgeOutsideCommentId : extraNode.rightEdgeOutsideCommentIdSet)
		{
			auto& extraComment = extraGraph.extraCommentList.at(rightEdgeOutsideCommentId);
			double minEdgeOutsidePosX = -INVALID_MIN_POS;
			for (size_t insideNodeId : extraComment.insideNodeIdSet)
			{
				placeNodeInMaxX(
					insideNodeId, visitedNodeIdSet, placedNodeIdSet, posMap, limitPosMap, diffMapMap, editableGraph, false);
				if (contains(placedNodeIdSet, insideNodeId))
				{
					// consider only placed nodes
					auto insideNode = graph.nodeList.at(insideNodeId);
					auto insideExtraNode = extraGraph.extraNodeList.at(insideNodeId);
					double commentDiffSpacingX = getCommentDiffSpacingX(extraNode, insideExtraNode, graphConfig);
					maxPosX = std::min(maxPosX,
						insideNode.box.Min.x - graphConfig.commentSpacing.x - commentDiffSpacingX - node.box.getSize().x);
					minEdgeOutsidePosX = std::min(minEdgeOutsidePosX, insideNode.box.Min.x);
				}
			}
			if (minEdgeOutsidePosX != -INVALID_MIN_POS)
				for (size_t insideNodeId : extraComment.insideNodeIdSet)
				{
					auto insideNode = graph.nodeList.at(insideNodeId);
					updatePosMap(limitPosMap, insideNode.id, minEdgeOutsidePosX, true);
				}
		}

		if (maxPosX != -INVALID_MIN_POS)
		{
			// place node
			MOVE_NODE(PLACE_NODE_IN_MAX_X, nodeId, Vector2(maxPosX, node.box.Min.y));
			placedNodeIdSet.insert(nodeId);

			// place loop children
			for (size_t loopChildId : extraNode.loopChildIdSet)
			{
				visitedNodeIdSet.insert(loopChildId);
				placeLoopChild(loopChildId, nodeId, editableGraph);
			}

			if (bRecursive)
				for (const auto& leftLinkIdList : {extraNode.GetLinkIdList(true, true), extraNode.GetLinkIdList(true, false)})
					for (const auto& leftLinkId : leftLinkIdList)
					{
						auto& leftExtraLink = extraGraph.extraLinkList.at(leftLinkId);
						if (leftExtraLink.isBroken) continue;
						auto& leftNode = leftExtraLink.GetNodeConnectedTo(graph, nodeId);
						// place left node
						placeNodeInMaxX(
							leftNode.id, visitedNodeIdSet, placedNodeIdSet, posMap, limitPosMap, diffMapMap, editableGraph, true);
					}
		}
		else

			if (!bRecursive)
			visitedNodeIdSet.erase(nodeId);
	}
};
