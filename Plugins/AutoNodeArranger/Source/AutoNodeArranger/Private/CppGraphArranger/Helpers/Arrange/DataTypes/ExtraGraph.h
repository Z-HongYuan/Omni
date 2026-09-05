// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <algorithm>
#include <cstdint>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include "../../../DataTypes/Graph/CppGraph.h"
#include "../../ExtendedCommon.h"
#include "./ExtraComment.h"
#include "./ExtraLink.h"
#include "./ExtraNode.h"

#include "../QDebug.h"

struct ExtraGraph
{
	// ======== ATTRIBUTES ========

	std::map<size_t, ExtraNode> extraNodeList;
	std::map<size_t, ExtraComment> extraCommentList;
	std::map<size_t, ExtraLink> extraLinkList;

	// ======== CONSTRUCTOR ========

	ExtraGraph() = default;
	// Be aware that this constructor contains a huge initialization cost
	ExtraGraph(const CppGraph& graph, bool bIncludeSubComment)
	{
		// Get node
		for (size_t i = 0; i < graph.nodeList.size(); i++)
		{
			ExtraNode extraNode;
			extraNodeList[i] = extraNode;
		}

		// Get comment id
		for (size_t i = 0; i < graph.commentList.size(); i++)
		{
			ExtraComment extraComment;
			extraComment.id = i;
			extraCommentList[i] = extraComment;
		}

		// Get link id
		for (size_t i = 0; i < graph.linkList.size(); i++)
		{
			ExtraLink extraLink;
			extraLink.id = i;
			extraLinkList[i] = extraLink;
		}

		// Get all links
		for (const auto& [_, link] : graph.linkList)
		{
			auto& extraLink = extraLinkList.at(link.id);
			extraLink.isExec = extraLink.GetPin(graph, true).isExec;
			extraLink.pinDiffY = extraLink.GetPin(graph, true).offset.y - extraLink.GetPin(graph, false).offset.y;
			auto leftNodId = extraLink.GetNode(graph, true).id;
			auto rightNodId = extraLink.GetNode(graph, false).id;
			auto& leftNode = extraNodeList.at(leftNodId);
			auto& rightNode = extraNodeList.at(rightNodId);
			extraLink.nodeDiffX = graph.nodeList.at(leftNodId).box.Min.x - graph.nodeList.at(rightNodId).box.Min.x;
			leftNode.GetLinkIdList(false, extraLink.isExec).push_back(link.id);
			rightNode.GetLinkIdList(true, extraLink.isExec).push_back(link.id);
		}

		// Get all nodes inside comments and comments containing nodes
		for (const auto& [_, comment] : graph.commentList)
			for (const auto& [__, node] : graph.nodeList)
				if (comment.box.intersectWith(node.box)) extraCommentList[comment.id].insideNodeIdSet.insert(node.id);

		if (bIncludeSubComment) includeSubComment(graph);

		// Get all comments containing nodes
		for (const auto& [_, extraComment] : extraCommentList)
			for (const auto& insideNodeId : extraComment.insideNodeIdSet)
				extraNodeList.at(insideNodeId).insideCommentIdSet.insert(extraComment.id);

		// Get all one output reroute nodes
		for (const auto& [_, node] : graph.nodeList)
		{
			auto& extraNode = extraNodeList.at(node.id);
			if (!node.isRerouteNode) continue;
			auto leftLinkCount = extraNode.GetCount({ELinkFlag::LeftExec, ELinkFlag::LeftNonExec});
			auto rightLinkCount = extraNode.GetCount({ELinkFlag::RightExec, ELinkFlag::RightNonExec});
			extraNode.isOneOutputRerouteNode = leftLinkCount == 1 && rightLinkCount == 1;
		}

		// Finish all links
		for (const auto& [_, link] : graph.linkList)
		{
			auto& extraLink = extraLinkList.at(link.id);
			auto leftNodId = extraLink.GetNode(graph, true).id;
			auto rightNodId = extraLink.GetNode(graph, false).id;
			extraLink.bFinishWithRerouteOneOutput = extraNodeList.at(rightNodId).isOneOutputRerouteNode;
			extraLink.bBetweenTwoRerouteOneOutput
				= extraNodeList.at(leftNodId).isOneOutputRerouteNode && extraNodeList.at(rightNodId).isOneOutputRerouteNode;
		}
	}

	// ======== METHODS ========

	// ExtraNode methods

	size_t GetFirstLinkId(size_t nodeId, std::initializer_list<ELinkFlag> linkFlags, bool bKeepBroken = false) const
	{
		for (auto linkIdList : extraNodeList.at(nodeId).GetLinkIdListList(linkFlags))
			for (size_t linkId : *linkIdList)
				if (bKeepBroken || !extraLinkList.at(linkId).isBroken) return linkId;
		return static_cast<size_t>(-1);
	}

	bool IsConnected(size_t nodeId, std::initializer_list<ELinkFlag> linkFlags) const
	{
		return GetFirstLinkId(nodeId, linkFlags) != static_cast<size_t>(-1);
	}

	// ExtraLink methods

	auto& GetExtraNode(size_t linkId, const CppGraph& graph, bool bLeft)
	{
		return extraNodeList[extraLinkList.at(linkId).GetNode(graph, bLeft).id];
	}

	auto& GetExtraNodeConnectedTo(size_t linkId, const CppGraph& graph, size_t fromNodeId)
	{
		return extraNodeList[extraLinkList.at(linkId).GetNodeConnectedTo(graph, fromNodeId).id];
	}

private:
	void includeSubComment(const CppGraph& graph)
	{
		// Sort all comment
		std::vector<ExtraComment*> sortedCommentList;
		for (auto& [_, extraComment] : extraCommentList) sortedCommentList.push_back(&extraComment);
		std::sort(sortedCommentList.begin(),
			sortedCommentList.end(),
			[&graph](const ExtraComment* lhs, const ExtraComment* rhs)
			{
				auto& lhsPos = graph.commentList.at(lhs->id).box.Min;
				auto& rhsPos = graph.commentList.at(rhs->id).box.Min;
				return lhs->insideNodeIdSet.size() != rhs->insideNodeIdSet.size()
						   ? lhs->insideNodeIdSet.size() > rhs->insideNodeIdSet.size()
					   : lhsPos.y != rhsPos.y ? lhsPos.y < rhsPos.y
											  : lhsPos.x < rhsPos.x;
			});
		// Distinct or include comments (modification in-place of extraCommentList)
		for (size_t i = 0; i < sortedCommentList.size(); i++)
		{
			auto commentI = sortedCommentList[i];
			for (size_t j = i + 1; j < sortedCommentList.size(); j++)
			{
				auto commentJ = sortedCommentList[j];
				size_t intersectSize = intersection(commentI->insideNodeIdSet, commentJ->insideNodeIdSet).size();
				if (intersectSize == 0) // distinct
					continue;
				commentI->insideCommentIdSet.insert(commentJ->id); // include the comment j into i

				if (intersectSize == commentJ->insideNodeIdSet.size()) // already included
					continue;
				for (auto jNodeID : commentJ->insideNodeIdSet)
					commentI->insideNodeIdSet.insert(jNodeID); // include the node of j not contained in i
				for (auto jCommentID : commentJ->insideCommentIdSet)
					commentI->insideCommentIdSet.insert(jCommentID); // include the comment of j not contained in i

				j = i; // reset search
			}
		}
	}
};
