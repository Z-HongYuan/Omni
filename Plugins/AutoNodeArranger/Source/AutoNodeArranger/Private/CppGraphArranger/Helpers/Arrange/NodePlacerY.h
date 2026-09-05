// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <functional>
#include <map>
#include <set>
#include <vector>

#include "../../DataTypes/Highlight.h"
#include "./NodePlacerCommon.h"

#define PLACE_FIRST_RIGHT_NODE_IN_Y
#define PLACE_LOOP_CHILD_IN_Y
#define PLACE_NODE_LEFT_NON_EXEC_IN_Y

class NodePlacerY
{
public:
	static void PlaceGraphInY(size_t root, EditableGraph& editableGraph)
	{
		std::list<NodePlaceInfo> nodePlaceInfoList;
		std::set<size_t> visitedNodeIdSet;
		std::set<size_t> placedNodeIdSet;
		std::set<size_t> partiallyPlacedNodeIdSet; // reroute chilren whose brother is not placed yet
		std::map<size_t, Box> placedLineMap;	   // key is linkId, value is LineBox
		std::vector<NodePlaceInfo> toPlaceNodePlaceInfoList;
		std::map<size_t, CommentPlaceInfo> placedCommentMap;		 // commentId -> number of nodes left to place
		std::map<size_t, std::set<size_t>> ignoredByCommentIdSetMap; // commentId -> set of commentId that ignore this comment
		std::vector<double> afterLeftRightAlignPosStack;

		for (size_t i = 0; i < editableGraph.graph.commentList.size(); i++) NodePlacerCommon::UpdateCommentBox(i, editableGraph);
		getNodeToPlaceExec(
			NodePlaceInfo(root, root, 0), visitedNodeIdSet, nodePlaceInfoList, editableGraph, static_cast<size_t>(-1));
		visitedNodeIdSet.clear();
		for (auto nodePlaceInfoIt = nodePlaceInfoList.cbegin(); nodePlaceInfoIt != nodePlaceInfoList.cend(); nodePlaceInfoIt++)
			getNodeToPlaceRight(false, *nodePlaceInfoIt, visitedNodeIdSet, nodePlaceInfoList, editableGraph, root);
		visitedNodeIdSet.clear();
		getAllNodeToPlace(NodePlaceInfo(root), visitedNodeIdSet, nodePlaceInfoList, editableGraph, root);
		visitedNodeIdSet.clear();
		placedNodeIdSet.insert(root);
		updatePlacedCommentMap(root, placedCommentMap, editableGraph.extraGraph);
		for (auto loopChildId : editableGraph.extraGraph.extraNodeList.at(root).loopChildIdSet)
		{
			placeLoopChildInY(loopChildId, root, placedNodeIdSet, partiallyPlacedNodeIdSet, placedLineMap, editableGraph);
			visitedNodeIdSet.insert(loopChildId);
			updatePlacedCommentMap(loopChildId, placedCommentMap, editableGraph.extraGraph);
		}

		for (const auto& nodePlaceInfo : nodePlaceInfoList)
		{
			if (nodePlaceInfo.nodeId != root && contains(placedNodeIdSet, nodePlaceInfo.nodeId)) continue;

			usePlaceFunction(nodePlaceInfo,
				visitedNodeIdSet,
				placedCommentMap,
				toPlaceNodePlaceInfoList,
				editableGraph,
				[&](const NodePlaceInfo& toPlaceNodePlaceInfo)
				{
					placeFirstRightNodeInY(true,
						toPlaceNodePlaceInfo.nodeId,
						visitedNodeIdSet,
						placedNodeIdSet,
						partiallyPlacedNodeIdSet,
						placedLineMap,
						placedCommentMap,
						ignoredByCommentIdSetMap,
						toPlaceNodePlaceInfoList,
						true,
						afterLeftRightAlignPosStack,
						toPlaceNodePlaceInfo.getPosY(editableGraph.graph),
						editableGraph);
				});

			usePlaceFunction(nodePlaceInfo,
				visitedNodeIdSet,
				placedCommentMap,
				toPlaceNodePlaceInfoList,
				editableGraph,
				[&](const NodePlaceInfo& toPlaceNodePlaceInfo)
				{
					placeNodeLeftNonExecInY(toPlaceNodePlaceInfo.nodeId,
						visitedNodeIdSet,
						placedNodeIdSet,
						partiallyPlacedNodeIdSet,
						placedLineMap,
						placedCommentMap,
						ignoredByCommentIdSetMap,
						toPlaceNodePlaceInfoList,
						true,
						afterLeftRightAlignPosStack,
						toPlaceNodePlaceInfo.getPosY(editableGraph.graph),
						editableGraph);
				});

			usePlaceFunction(nodePlaceInfo,
				visitedNodeIdSet,
				placedCommentMap,
				toPlaceNodePlaceInfoList,
				editableGraph,
				[&](const NodePlaceInfo& toPlaceNodePlaceInfo)
				{
					placeFirstRightNodeInY(false,
						toPlaceNodePlaceInfo.nodeId,
						visitedNodeIdSet,
						placedNodeIdSet,
						partiallyPlacedNodeIdSet,
						placedLineMap,
						placedCommentMap,
						ignoredByCommentIdSetMap,
						toPlaceNodePlaceInfoList,
						true,
						afterLeftRightAlignPosStack,
						toPlaceNodePlaceInfo.getPosY(editableGraph.graph),
						editableGraph);
				});
		}
	}

private:
	static double getCommentDiffSpacingY(
		const ExtraNode& extraNode, const ExtraNode& belowExtraNode, const CppGraphConfig& graphConfig)
	{
		auto extraToBelowDiffInside = difference(extraNode.insideCommentIdSet, belowExtraNode.insideCommentIdSet);
		auto belowToExtraDiffInside = difference(belowExtraNode.insideCommentIdSet, extraNode.insideCommentIdSet);
		auto commentDiffCount = extraToBelowDiffInside.size() + belowToExtraDiffInside.size();
		return graphConfig.commentSpacing.y * commentDiffCount + belowToExtraDiffInside.size() * COMMENT_HEADER.y;
	}

	struct CommentPlaceInfo
	{
		CommentPlaceInfo() = default;
		explicit CommentPlaceInfo(size_t remainingNodeCount_) : remainingNodeCount(remainingNodeCount_) {}

		size_t remainingNodeCount = static_cast<size_t>(-1);
	};

	static bool getIsAllLeftRightCommentPlaced(
		size_t nodeId, const std::map<size_t, CommentPlaceInfo>& placedCommentMap, const ExtraGraph& extraGraph)
	{
		auto& extraNode = extraGraph.extraNodeList.at(nodeId);
		for (size_t leftRightCommentId : extraNode.leftRightCommentIdSet)
		{
			auto placedCommentMapIt = placedCommentMap.find(leftRightCommentId);
			if (placedCommentMapIt == placedCommentMap.end() || placedCommentMapIt->second.remainingNodeCount != 0) return false;
		}
		return true;
	}

	struct NodePlaceInfo
	{
		NodePlaceInfo() = default;
		// constructor with one parameter for INVALID_MIN_POS
		explicit NodePlaceInfo(size_t nodeId_) : nodeId(nodeId_), referenceNodeId(static_cast<size_t>(-1)), diffPosY(0) {}
		NodePlaceInfo(size_t nodeId_, size_t referenceNodeId_, double diffPosY_) :
			nodeId(nodeId_), referenceNodeId(referenceNodeId_), diffPosY(diffPosY_)
		{
		}

		double getPosY(const CppGraph& cppGraph) const
		{
			if (referenceNodeId == static_cast<size_t>(-1)) return INVALID_MIN_POS;
			auto& referenceNode = cppGraph.nodeList.at(referenceNodeId);
			return referenceNode.box.Min.y + diffPosY;
		}

		size_t nodeId;
		size_t referenceNodeId;
		double diffPosY;
	};

	using PlaceFunction = std::function<void(const NodePlaceInfo& toPlaceNodePlaceInfo)>;

	static void usePlaceFunction(const NodePlaceInfo& nodePlaceInfo,
		std::set<size_t>& visitedNodeIdSet,
		std::map<size_t, CommentPlaceInfo>& placedCommentMap,
		std::vector<NodePlaceInfo>& toPlaceNodePlaceInfoList,
		EditableGraph& editableGraph,
		PlaceFunction placeFunction)
	{
		visitedNodeIdSet.clear();
		toPlaceNodePlaceInfoList.push_back(nodePlaceInfo);
		while (!toPlaceNodePlaceInfoList.empty())
		{
			std::vector<NodePlaceInfo> newlyToPlaceNodePlaceInfoList = copy_if_vector<NodePlaceInfo>(toPlaceNodePlaceInfoList,
				[&](const NodePlaceInfo& toPlaceNodePlaceInfo)
				{
					return getIsAllLeftRightCommentPlaced(
						toPlaceNodePlaceInfo.nodeId, placedCommentMap, editableGraph.extraGraph);
				}); // make a filtered copy
			for (const NodePlaceInfo& newlyToPlaceNodePlaceInfo : newlyToPlaceNodePlaceInfoList)
				visitedNodeIdSet.erase(newlyToPlaceNodePlaceInfo.nodeId);
			toPlaceNodePlaceInfoList.clear();
			for (const NodePlaceInfo& toPlaceNodePlaceInfo : newlyToPlaceNodePlaceInfoList) placeFunction(toPlaceNodePlaceInfo);
		}
	}

	static void getNodeToPlaceExec(const NodePlaceInfo& nodePlaceInfo,
		std::set<size_t>& visitedNodeIdSet,
		std::list<NodePlaceInfo>& nodePlaceInfoList,
		EditableGraph& editableGraph,
		size_t root)
	{
		if (contains(visitedNodeIdSet, nodePlaceInfo.nodeId)) return;
		visitedNodeIdSet.insert(nodePlaceInfo.nodeId);
		if (nodePlaceInfo.nodeId != root) nodePlaceInfoList.push_back(nodePlaceInfo);

		auto& graph = editableGraph.graph;
		auto& extraGraph = editableGraph.extraGraph;
		auto& extraNode = extraGraph.extraNodeList.at(nodePlaceInfo.nodeId);

		for (size_t rightLinkId : extraNode.GetLinkIdList(false, true))
		{
			auto& extraLink = extraGraph.extraLinkList.at(rightLinkId);
			if (extraLink.isBroken) continue;
			auto& rightNode = extraLink.GetNodeConnectedTo(graph, nodePlaceInfo.nodeId);
			getNodeToPlaceExec(NodePlaceInfo(rightNode.id, nodePlaceInfo.nodeId, extraLink.pinDiffY),
				visitedNodeIdSet,
				nodePlaceInfoList,
				editableGraph,
				root);
		}

		for (size_t leftExecLinkId : extraNode.GetLinkIdList(true, true))
		{
			auto& extraLink = extraGraph.extraLinkList.at(leftExecLinkId);
			if (extraLink.isBroken) continue;
			auto& leftNode = extraLink.GetNodeConnectedTo(graph, nodePlaceInfo.nodeId);
			getNodeToPlaceLeftExec(
				NodePlaceInfo(leftNode.id, nodePlaceInfo.referenceNodeId, nodePlaceInfo.diffPosY - extraLink.pinDiffY),
				visitedNodeIdSet,
				nodePlaceInfoList,
				editableGraph,
				root);
		}
	}

	static void getNodeToPlaceLeftExec(const NodePlaceInfo& nodePlaceInfo,
		std::set<size_t>& visitedNodeIdSet,
		std::list<NodePlaceInfo>& nodePlaceInfoList,
		EditableGraph& editableGraph,
		size_t root)
	{
		editableGraph.CheckTimeout();
		if (contains(visitedNodeIdSet, nodePlaceInfo.nodeId)) return;

		auto& graph = editableGraph.graph;
		auto& extraGraph = editableGraph.extraGraph;
		auto& extraNode = extraGraph.extraNodeList.at(nodePlaceInfo.nodeId);

		if (extraGraph.IsConnected(nodePlaceInfo.nodeId, {ELinkFlag::LeftExec}))
		{
			for (size_t leftExecLinkId : extraNode.GetLinkIdList(true, true))
			{
				auto& extraLink = extraGraph.extraLinkList.at(leftExecLinkId);
				if (extraLink.isBroken) continue;
				auto& leftNode = extraLink.GetNodeConnectedTo(graph, nodePlaceInfo.nodeId);
				getNodeToPlaceLeftExec(
					NodePlaceInfo(leftNode.id, nodePlaceInfo.referenceNodeId, nodePlaceInfo.diffPosY - extraLink.pinDiffY),
					visitedNodeIdSet,
					nodePlaceInfoList,
					editableGraph,
					root);
			}
		}
		else
			getNodeToPlaceExec(nodePlaceInfo, visitedNodeIdSet, nodePlaceInfoList, editableGraph, root);
	}

	static void getNodeToPlaceRight(bool bExec,
		const NodePlaceInfo& nodePlaceInfo,
		std::set<size_t>& visitedNodeIdSet,
		std::list<NodePlaceInfo>& nodePlaceInfoList,
		EditableGraph& editableGraph,
		size_t root)
	{
		editableGraph.CheckTimeout();
		if (contains(visitedNodeIdSet, nodePlaceInfo.nodeId)) return;
		visitedNodeIdSet.insert(nodePlaceInfo.nodeId);
		if (nodePlaceInfo.nodeId != root) nodePlaceInfoList.push_back(nodePlaceInfo);

		auto& graph = editableGraph.graph;
		auto& extraGraph = editableGraph.extraGraph;
		auto& extraNode = extraGraph.extraNodeList.at(nodePlaceInfo.nodeId);

		for (size_t rightLinkId : extraNode.GetLinkIdList(false, bExec))
		{
			auto& extraLink = extraGraph.extraLinkList.at(rightLinkId);
			if (extraLink.isBroken) continue;
			auto& rightNode = extraLink.GetNodeConnectedTo(graph, nodePlaceInfo.nodeId);
			getNodeToPlaceRight(bExec,
				NodePlaceInfo(rightNode.id, nodePlaceInfo.nodeId, extraLink.pinDiffY),
				visitedNodeIdSet,
				nodePlaceInfoList,
				editableGraph,
				root);
		}
	}

	// small fix up
	static void getAllNodeToPlace(const NodePlaceInfo& nodePlaceInfo,
		std::set<size_t>& visitedNodeIdSet,
		std::list<NodePlaceInfo>& nodePlaceInfoList,
		EditableGraph& editableGraph,
		size_t root)
	{
		editableGraph.CheckTimeout();
		if (contains(visitedNodeIdSet, nodePlaceInfo.nodeId)) return;
		visitedNodeIdSet.insert(nodePlaceInfo.nodeId);
		if (nodePlaceInfo.nodeId != root) nodePlaceInfoList.push_back(nodePlaceInfo);

		auto& graph = editableGraph.graph;
		auto& extraGraph = editableGraph.extraGraph;
		auto& extraNode = extraGraph.extraNodeList.at(nodePlaceInfo.nodeId);

		for (auto rightLinkIdList : extraNode.GetLinkIdListList({ELinkFlag::RightExec, ELinkFlag::RightNonExec}))
		{
			for (size_t rightLinkId : *rightLinkIdList)
			{
				auto& extraLink = extraGraph.extraLinkList.at(rightLinkId);
				if (extraLink.isBroken) continue;
				auto& rightNode = extraLink.GetNodeConnectedTo(graph, nodePlaceInfo.nodeId);
				getAllNodeToPlace(NodePlaceInfo(rightNode.id, nodePlaceInfo.nodeId, extraLink.pinDiffY),
					visitedNodeIdSet,
					nodePlaceInfoList,
					editableGraph,
					root);
			}
		}

		for (auto leftLinkIdList : extraNode.GetLinkIdListList({ELinkFlag::LeftExec, ELinkFlag::LeftNonExec}))
		{
			for (size_t leftLinkId : *leftLinkIdList)
			{
				auto& extraLink = extraGraph.extraLinkList.at(leftLinkId);
				if (extraLink.isBroken) continue;
				auto& leftNode = extraLink.GetNodeConnectedTo(graph, nodePlaceInfo.nodeId);
				getAllNodeToPlace(
					NodePlaceInfo(leftNode.id, nodePlaceInfo.referenceNodeId, nodePlaceInfo.diffPosY - extraLink.pinDiffY),
					visitedNodeIdSet,
					nodePlaceInfoList,
					editableGraph,
					root);
			}
		}
	}

	static void updatePlacedCommentMap(
		size_t nodeId, std::map<size_t, CommentPlaceInfo>& placedCommentMap, const ExtraGraph& extraGraph)
	{
		auto& extraNode = extraGraph.extraNodeList.at(nodeId);
		for (auto& insideCommentId : extraNode.insideCommentIdSet)
		{
			auto placedCommentMapIt = placedCommentMap.find(insideCommentId);
			auto& extraComment = extraGraph.extraCommentList.at(insideCommentId);
			if (placedCommentMapIt == placedCommentMap.end())
				placedCommentMap[insideCommentId] = CommentPlaceInfo(extraComment.insideNodeIdSet.size() - 1);
			else
				placedCommentMapIt->second.remainingNodeCount--;
		}
	}

	static double handleOverlap(size_t nodeId,
		const std::set<size_t>& placedNodeIdSet,
		const std::map<size_t, Box>& placedLineMap,
		double alignPosY, // position used to align a node with previous node
		const EditableGraph& editableGraph)
	{
		auto& graph = editableGraph.graph;
		auto& graphConfig = graph.graphConfig;
		auto& extraGraph = editableGraph.extraGraph;
		auto& node = graph.nodeList.at(nodeId);
		auto& extraNode = extraGraph.extraNodeList.at(nodeId);

		for (size_t placedNodeId : placedNodeIdSet)
		{
			auto& placedNode = graph.nodeList.at(placedNodeId);
			auto& extraPlacedNode = extraGraph.extraNodeList.at(placedNodeId);
			bool bIntersectInX = placedNode.box.intersectWithInX(node.box);
			for (size_t insideCommentId : extraNode.insideCommentIdSet)
			{
				if (bIntersectInX) break;
				auto& comment = graph.commentList.at(insideCommentId);
				auto& extraComment = extraGraph.extraCommentList.at(insideCommentId);
				if (contains(extraComment.insideNodeIdSet, placedNodeId)) continue;
				bIntersectInX = comment.box.intersectWithInX(placedNode.box);
			}
			for (size_t insidePlacedCommentId : extraPlacedNode.insideCommentIdSet)
			{
				if (bIntersectInX) break;
				auto& placedComment = graph.commentList.at(insidePlacedCommentId);
				auto& extraPlacedComment = extraGraph.extraCommentList.at(insidePlacedCommentId);
				if (contains(extraPlacedComment.insideNodeIdSet, nodeId)) continue;
				bIntersectInX = placedComment.box.intersectWithInX(node.box);
			}
			if (bIntersectInX)
			{
				double commentDiffY = getCommentDiffSpacingY(extraPlacedNode, extraNode, graphConfig);
				alignPosY = std::max(alignPosY, placedNode.box.Max.y + graphConfig.spacing.y + commentDiffY);
			}
		}
		Box nodeBox = node.box;
		auto nodeSize = node.box.getSize();
		nodeBox.Min.y = alignPosY;
		nodeBox.Max.y = alignPosY + nodeSize.y;
		for (auto& [_, placedLineBox] : placedLineMap)
		{
			if (placedLineBox.intersectWith(nodeBox))
			{
				alignPosY += 2 * graphConfig.lineWidth;
				nodeBox = nodeBox.offsetBox(Vector2(0, 2 * graphConfig.lineWidth));
			}
		}
		return alignPosY;
	}

	static void placeFirstRightNodeInY(bool bExec,
		size_t nodeId,
		std::set<size_t>& visitedNodeIdSet,
		std::set<size_t>& placedNodeIdSet,
		std::set<size_t>& partiallyPlacedNodeIdSet,
		std::map<size_t, Box>& placedLineMap,
		std::map<size_t, CommentPlaceInfo>& placedCommentMap,
		std::map<size_t, std::set<size_t>>& ignoredByCommentIdSetMap,
		std::vector<NodePlaceInfo>& toPlaceNodePlaceInfoList,
		bool isToPlace,
		// stack of positions used to place a node after left right nodes
		// (pushed when next node is left right node, popped when placed)
		std::vector<double>& afterLeftRightAlignPosStack,
		double alignPosY, // position used to align a node with previous node
		EditableGraph& editableGraph)
	{
		editableGraph.CheckTimeout();
		if (contains(visitedNodeIdSet, nodeId)) return;
		visitedNodeIdSet.insert(nodeId);
		auto& graph = editableGraph.graph;
		auto& graphConfig = graph.graphConfig;
		auto& extraGraph = editableGraph.extraGraph;
		auto& node = graph.nodeList.at(nodeId);
		auto& extraNode = extraGraph.extraNodeList.at(nodeId);

		auto allLeftRightCommentIdSet = extraNode.leftRightCommentIdSet;
		for (size_t insideCommentId : extraNode.insideCommentIdSet)
		{
			auto& insideExtraComment = extraGraph.extraCommentList[insideCommentId];
			allLeftRightCommentIdSet.insert(
				insideExtraComment.leftRightCommentIdSet.begin(), insideExtraComment.leftRightCommentIdSet.end());
		}

		// update ignoredByCommentIdSetMap
		// (comments containing current node should either ignore left right comments or be ignored by them)
		for (size_t insideCommentId : extraNode.insideCommentIdSet)
		{
			for (size_t leftRightCommentId : allLeftRightCommentIdSet)
			{
				auto insideCommentIt = ignoredByCommentIdSetMap.find(insideCommentId);
				if (insideCommentIt != ignoredByCommentIdSetMap.end() && contains(insideCommentIt->second, leftRightCommentId))
					continue;

				auto leftRightCommentIt = ignoredByCommentIdSetMap.find(leftRightCommentId);
				if (leftRightCommentIt == ignoredByCommentIdSetMap.end())
					ignoredByCommentIdSetMap[leftRightCommentId] = {insideCommentId};
				else
					leftRightCommentIt->second.insert(insideCommentId);

				auto& insideComment = graph.commentList[insideCommentId];
				auto& leftRightExtraComment = extraGraph.extraCommentList[leftRightCommentId];
				bool bIntersectLeftRightCommentInX = false;
				for (auto insideNodeId : leftRightExtraComment.insideNodeIdSet)
				{
					auto& insideNode = graph.nodeList.at(insideNodeId);
					if (insideNode.box.intersectWithInX(insideComment.box))
					{
						bIntersectLeftRightCommentInX = true;
						break;
					}
				}
				if (!bIntersectLeftRightCommentInX)
				{
					// if no intersection in X, then both comments should ignore each other
					if (insideCommentIt == ignoredByCommentIdSetMap.end())
						ignoredByCommentIdSetMap[insideCommentId] = {leftRightCommentId};
					else
						insideCommentIt->second.insert(leftRightCommentId);
				}
			}
		}

		// check if all left right comments are placed and update alignPosY
		for (size_t leftRightCommentId : allLeftRightCommentIdSet)
		{
			auto ignoredByCommentIdSetMapIt = ignoredByCommentIdSetMap.find(leftRightCommentId);
			if (ignoredByCommentIdSetMapIt != ignoredByCommentIdSetMap.end()
				&& !intersection(ignoredByCommentIdSetMapIt->second, extraNode.insideCommentIdSet).empty())
			{
				bool isNodeLeftRightComment = contains(extraNode.leftRightCommentIdSet, leftRightCommentId);
				if (!isNodeLeftRightComment)
					continue; // continue done here to avoid placing node in a comment that should be ignored

				// check if comment is symmetrically ignored by all inside comments
				bool isSymmetricIgnore = true;
				for (size_t insideCommentId : extraNode.insideCommentIdSet)
				{
					auto insideCommentIt = ignoredByCommentIdSetMap.find(insideCommentId);
					if (insideCommentIt == ignoredByCommentIdSetMap.end()
						|| !contains(insideCommentIt->second, leftRightCommentId))
					{
						isSymmetricIgnore = false;
						break;
					}
				}
				if (!isSymmetricIgnore) continue; // continue done here to avoid placing node in a comment that should be ignored
			}
			auto placedCommentMapIt = placedCommentMap.find(leftRightCommentId);
			auto extraComment = extraGraph.extraCommentList[leftRightCommentId];
			bool isCommentPlaced
				= placedCommentMapIt != placedCommentMap.end() && placedCommentMapIt->second.remainingNodeCount == 0;
			isToPlace &= isCommentPlaced;
			if (!isToPlace) continue; // avoid heavy computation if not needed
			for (auto& insideNodeId : extraComment.insideNodeIdSet)
			{
				auto& insideNode = graph.nodeList.at(insideNodeId);
				auto& insideExtraNode = extraGraph.extraNodeList.at(insideNodeId);
				double commentDiffY = getCommentDiffSpacingY(insideExtraNode, extraNode, graphConfig);
				alignPosY = std::max(alignPosY, insideNode.box.Max.y + graphConfig.spacing.y + commentDiffY);
			}
		}

		bool bAlreadyPlaced = contains(placedNodeIdSet, nodeId);
		if (bAlreadyPlaced) alignPosY = node.box.Min.y;
		else
			alignPosY = handleOverlap(nodeId, placedNodeIdSet, placedLineMap, alignPosY, editableGraph);

		if (!isToPlace) toPlaceNodePlaceInfoList.emplace_back(nodeId);

		// place first right node
		auto firstRightLinkId = extraGraph.GetFirstLinkId(nodeId, {bExec ? ELinkFlag::RightExec : ELinkFlag::RightNonExec});
		if (firstRightLinkId != static_cast<size_t>(-1))
		{
			auto& extraLink = extraGraph.extraLinkList.at(firstRightLinkId);
			auto& rightNode = extraLink.GetNodeConnectedTo(graph, nodeId);
			auto& rightExtraNode = extraGraph.extraNodeList.at(rightNode.id);
			if (!contains(placedNodeIdSet, rightNode.id)) // do nothing if node already placed
			{
				double newAlignPosY = alignPosY + extraLink.pinDiffY;
				bool bEnterOrExitLeftRightComment = false;
				if (!intersection(extraNode.leftRightCommentIdSet, rightExtraNode.insideCommentIdSet).empty())
				{
					// entering back to the comment --> use pos stored in afterLeftRightAlignPosStack
					if (!afterLeftRightAlignPosStack.empty())
					{
						newAlignPosY = afterLeftRightAlignPosStack.back();
						afterLeftRightAlignPosStack.pop_back();
					}
					bEnterOrExitLeftRightComment = true;
				}
				if (!intersection(extraNode.insideCommentIdSet, rightExtraNode.leftRightCommentIdSet).empty())
				{
					afterLeftRightAlignPosStack.push_back(alignPosY);
					bEnterOrExitLeftRightComment = true;
				}
				bool newIsToPlace = isToPlace || bEnterOrExitLeftRightComment;
				placeFirstRightNodeInY(bExec,
					rightNode.id,
					visitedNodeIdSet,
					placedNodeIdSet,
					partiallyPlacedNodeIdSet,
					placedLineMap,
					placedCommentMap,
					ignoredByCommentIdSetMap,
					toPlaceNodePlaceInfoList,
					newIsToPlace,
					afterLeftRightAlignPosStack,
					newAlignPosY,
					editableGraph);
				if (!bEnterOrExitLeftRightComment && contains(placedNodeIdSet, rightNode.id))
				{
					// update back alignPosY
					alignPosY = rightNode.box.Min.y - extraLink.pinDiffY;
					Box linkBox;
					NodePlacerCommon::GetLinkBox(linkBox, firstRightLinkId, editableGraph, alignPosY, false, true);
					placedLineMap[firstRightLinkId] = linkBox;
				}
			}
		}

		if (isToPlace && !bAlreadyPlaced)
		{
			// place node or update alignPosY
			if (alignPosY == INVALID_MIN_POS) throw std::runtime_error("alignPosY is invalid");
			MOVE_NODE(PLACE_FIRST_RIGHT_NODE_IN_Y, nodeId, Vector2(node.box.Min.x, alignPosY));
			placedNodeIdSet.insert(nodeId);

			// place reroute children
			for (size_t loopChildId : extraNode.loopChildIdSet)
			{
				placeLoopChildInY(loopChildId, nodeId, placedNodeIdSet, partiallyPlacedNodeIdSet, placedLineMap, editableGraph);
				visitedNodeIdSet.insert(loopChildId);
				updatePlacedCommentMap(loopChildId, placedCommentMap, extraGraph);
			}

			updatePlacedCommentMap(nodeId, placedCommentMap, extraGraph);
		}
	}

	static void placeLoopChildInY(size_t childId,
		size_t parentId,
		std::set<size_t>& placedNodeIdSet,
		std::set<size_t>& partiallyPlacedNodeIdSet,
		const std::map<size_t, Box>& placedLineMap,
		EditableGraph& editableGraph)
	{
		if (contains(placedNodeIdSet, childId)) return;
		auto& graph = editableGraph.graph;
		auto& extraGraph = editableGraph.extraGraph;
		auto& extraChild = extraGraph.extraNodeList.at(childId);
		auto& parentNode = graph.nodeList.at(parentId);
		auto& childNode = graph.nodeList.at(childId);
		double posY = parentNode.box.Max.y + graph.graphConfig.spacing.y;
		auto& onRightChildId = extraChild.isLoopBrotherPlacedOnRight ? childId : extraChild.loopBrotherId;
		size_t firstLeftLinkId = extraGraph.GetFirstLinkId(onRightChildId, {ELinkFlag::Left}, true);
		if (firstLeftLinkId == static_cast<size_t>(-1))
			throw std::runtime_error("firstLeftLinkId is invalid"); // should not happen

		if (contains(partiallyPlacedNodeIdSet, extraChild.loopBrotherId))
		{
			auto& loopBrotherNode = graph.nodeList.at(extraChild.loopBrotherId);
			posY = std::max(posY, loopBrotherNode.box.Min.y);
			double oldPosY = posY + 1;
			while (oldPosY != posY)
			{
				oldPosY = posY;
				posY = handleOverlap(childId, placedNodeIdSet, placedLineMap, posY, editableGraph);
				posY = handleOverlap(loopBrotherNode.id, placedNodeIdSet, placedLineMap, posY, editableGraph);
			}
			MOVE_NODE(PLACE_LOOP_CHILD_IN_Y, loopBrotherNode.id, Vector2(loopBrotherNode.box.Min.x, posY));
			partiallyPlacedNodeIdSet.erase(extraChild.loopBrotherId);
			placedNodeIdSet.insert(extraChild.loopBrotherId);
		}
		else
			partiallyPlacedNodeIdSet.insert(childId);
		MOVE_NODE(PLACE_LOOP_CHILD_IN_Y, childNode.id, Vector2(childNode.box.Min.x, posY));
	}

	static void placeNodeLeftNonExecInY(size_t nodeId,
		std::set<size_t>& visitedNodeIdSet,
		std::set<size_t>& placedNodeIdSet,
		std::set<size_t>& partiallyPlacedNodeIdSet,
		std::map<size_t, Box>& placedLineMap,
		std::map<size_t, CommentPlaceInfo>& placedCommentMap,
		std::map<size_t, std::set<size_t>>& ignoredByCommentIdSetMap,
		std::vector<NodePlaceInfo>& toPlaceNodePlaceInfoList,
		bool isToPlace,
		// stack of positions used to place a node after left right nodes
		// (pushed when next node is left right node, popped when placed)
		std::vector<double>& afterLeftRightAlignPosStack,
		double alignPosY, // position used to align a node with previous node
		EditableGraph& editableGraph)
	{
		editableGraph.CheckTimeout();
		if (contains(visitedNodeIdSet, nodeId)) return;
		visitedNodeIdSet.insert(nodeId);
		auto& graph = editableGraph.graph;
		auto& graphConfig = graph.graphConfig;
		auto& extraGraph = editableGraph.extraGraph;
		auto& node = graph.nodeList.at(nodeId);
		auto& extraNode = extraGraph.extraNodeList.at(nodeId);

		auto allLeftRightCommentIdSet = extraNode.leftRightCommentIdSet;
		for (size_t insideCommentId : extraNode.insideCommentIdSet)
		{
			auto& insideExtraComment = extraGraph.extraCommentList[insideCommentId];
			allLeftRightCommentIdSet.insert(
				insideExtraComment.leftRightCommentIdSet.begin(), insideExtraComment.leftRightCommentIdSet.end());
		}

		// update ignoredByCommentIdSetMap
		// (comments containing current node should either ignore left right comments or be ignored by them)
		for (size_t insideCommentId : extraNode.insideCommentIdSet)
		{
			for (size_t leftRightCommentId : allLeftRightCommentIdSet)
			{
				auto leftRightCommentIt = ignoredByCommentIdSetMap.find(insideCommentId);
				if (leftRightCommentIt != ignoredByCommentIdSetMap.end()
					&& contains(leftRightCommentIt->second, leftRightCommentId))
					continue;
				auto insideCommentIt = ignoredByCommentIdSetMap.find(leftRightCommentId);
				if (insideCommentIt == ignoredByCommentIdSetMap.end())
					ignoredByCommentIdSetMap[leftRightCommentId] = {insideCommentId};
				else
					insideCommentIt->second.insert(insideCommentId);
			}
		}

		// check if all left right comments are placed and update alignPosY
		for (size_t leftRightCommentId : allLeftRightCommentIdSet)
		{
			auto ignoredByCommentIdSetMapIt = ignoredByCommentIdSetMap.find(leftRightCommentId);
			if (ignoredByCommentIdSetMapIt != ignoredByCommentIdSetMap.end()
				&& !intersection(ignoredByCommentIdSetMapIt->second, extraNode.insideCommentIdSet).empty())
				continue; // continue done here to avoid placing node in a comment that should be ignored
			auto placedCommentMapIt = placedCommentMap.find(leftRightCommentId);
			auto extraComment = extraGraph.extraCommentList[leftRightCommentId];
			bool isCommentPlaced
				= placedCommentMapIt != placedCommentMap.end() && placedCommentMapIt->second.remainingNodeCount == 0;
			isToPlace &= isCommentPlaced;
			if (!isToPlace) continue; // avoid heavy computation if not needed
			for (auto& insideNodeId : extraComment.insideNodeIdSet)
			{
				auto& insideNode = graph.nodeList.at(insideNodeId);
				auto& insideExtraNode = extraGraph.extraNodeList.at(insideNodeId);
				double commentDiffY = getCommentDiffSpacingY(insideExtraNode, extraNode, graphConfig);
				alignPosY = std::max(alignPosY, insideNode.box.Max.y + graphConfig.spacing.y + commentDiffY);
			}
		}

		bool bAlreadyPlaced = contains(placedNodeIdSet, nodeId);

		if (bAlreadyPlaced) alignPosY = node.box.Min.y;
		else
			alignPosY = handleOverlap(nodeId, placedNodeIdSet, placedLineMap, alignPosY, editableGraph);

		// place first left non exec node
		size_t firstLeftNonExecLinkId
			= bAlreadyPlaced ? static_cast<size_t>(-1) : extraGraph.GetFirstLinkId(nodeId, {ELinkFlag::LeftNonExec});
		if (firstLeftNonExecLinkId != static_cast<size_t>(-1))
		{
			auto& extraLink = extraGraph.extraLinkList.at(firstLeftNonExecLinkId);
			auto& leftNode = extraLink.GetNodeConnectedTo(graph, nodeId);
			auto& leftExtraNode = extraGraph.extraNodeList.at(leftNode.id);
			if (!extraLink.isBroken && !contains(placedNodeIdSet, leftNode.id)) // do nothing if node already placed
			{
				double newAlignPosY = alignPosY - extraLink.pinDiffY;
				bool bEnterOrExitLeftRightComment = false;
				if (!intersection(extraNode.leftRightCommentIdSet, leftExtraNode.insideCommentIdSet).empty())
				{
					// entering back to the comment --> use pos stored in afterLeftRightAlignPosStack
					if (!afterLeftRightAlignPosStack.empty())
					{
						newAlignPosY = afterLeftRightAlignPosStack.back();
						afterLeftRightAlignPosStack.pop_back();
					}
					bEnterOrExitLeftRightComment = true;
				}
				if (!intersection(extraNode.insideCommentIdSet, leftExtraNode.leftRightCommentIdSet).empty())
				{
					afterLeftRightAlignPosStack.push_back(alignPosY);
					bEnterOrExitLeftRightComment = true;
				}
				bool newIsToPlace = isToPlace || bEnterOrExitLeftRightComment;
				placeNodeLeftNonExecInY(leftNode.id,
					visitedNodeIdSet,
					placedNodeIdSet,
					partiallyPlacedNodeIdSet,
					placedLineMap,
					placedCommentMap,
					ignoredByCommentIdSetMap,
					toPlaceNodePlaceInfoList,
					newIsToPlace,
					afterLeftRightAlignPosStack,
					newAlignPosY,
					editableGraph);
				if (!bEnterOrExitLeftRightComment && contains(placedNodeIdSet, leftNode.id))
				{
					// update back alignPosY
					alignPosY = leftNode.box.Min.y + extraLink.pinDiffY;
					Box linkBox;
					NodePlacerCommon::GetLinkBox(linkBox, firstLeftNonExecLinkId, editableGraph, alignPosY, false, true);
					placedLineMap[firstLeftNonExecLinkId] = linkBox;
				}
			}
		}

		if (isToPlace)
		{
			// place node or update alignPosY
			if (!bAlreadyPlaced)
			{
				if (alignPosY == INVALID_MIN_POS) throw std::runtime_error("alignPosY is invalid");
				MOVE_NODE(PLACE_NODE_LEFT_NON_EXEC_IN_Y, nodeId, Vector2(node.box.Min.x, alignPosY));
				placedNodeIdSet.insert(nodeId);

				// place reroute children
				for (size_t loopChildId : extraNode.loopChildIdSet)
				{
					placeLoopChildInY(
						loopChildId, nodeId, placedNodeIdSet, partiallyPlacedNodeIdSet, placedLineMap, editableGraph);
					visitedNodeIdSet.insert(loopChildId);
					updatePlacedCommentMap(loopChildId, placedCommentMap, extraGraph);
				}

				updatePlacedCommentMap(nodeId, placedCommentMap, extraGraph);
			}

			// place all other left non exec node
			for (size_t leftLinkId : extraNode.GetLinkIdList(true, false))
			{
				if (firstLeftNonExecLinkId == leftLinkId) continue;
				auto& extraLink = extraGraph.extraLinkList.at(leftLinkId);
				if (extraLink.isBroken) continue;
				auto& leftNode = extraLink.GetNodeConnectedTo(graph, nodeId);
				double newAlignPosY = alignPosY - extraLink.pinDiffY;
				placeNodeLeftNonExecInY(leftNode.id,
					visitedNodeIdSet,
					placedNodeIdSet,
					partiallyPlacedNodeIdSet,
					placedLineMap,
					placedCommentMap,
					ignoredByCommentIdSetMap,
					toPlaceNodePlaceInfoList,
					true,
					afterLeftRightAlignPosStack,
					newAlignPosY,
					editableGraph);
			}
		}
		else
			toPlaceNodePlaceInfoList.emplace_back(nodeId);

		// if already placed and not yet visited ==> go on first right exec node
		if (bAlreadyPlaced)
		{
			size_t firstRightExecLinkId = extraGraph.GetFirstLinkId(nodeId, {ELinkFlag::RightExec});
			if (firstRightExecLinkId != static_cast<size_t>(-1))
			{
				auto& extraLink = extraGraph.extraLinkList.at(firstRightExecLinkId);
				auto& rightNode = extraLink.GetNodeConnectedTo(graph, nodeId);
				placeNodeLeftNonExecInY(rightNode.id,
					visitedNodeIdSet,
					placedNodeIdSet,
					partiallyPlacedNodeIdSet,
					placedLineMap,
					placedCommentMap,
					ignoredByCommentIdSetMap,
					toPlaceNodePlaceInfoList,
					true,
					afterLeftRightAlignPosStack,
					alignPosY + extraLink.pinDiffY,
					editableGraph);
			}
		}
	}
};
