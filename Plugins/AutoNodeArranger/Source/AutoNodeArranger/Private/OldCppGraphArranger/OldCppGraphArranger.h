// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <fstream>

#include "../CppGraphArranger/DataTypes/Highlight.h"
#include "_ExtendedObjects/ExtendedGraph.h"

class OldCppGraphArranger
{
public:
	static GraphInstruction ArrangeGraph(
		const CppGraph& cppGraph, const std::vector<size_t> selectedNodeIDList = std::vector<size_t>())
	{
		globalHiglights.clear();

		if (!cppGraph.graphConfig.bUseArrangement) return GraphInstruction();

		ExtendedGraph extendedGraph;
		extendedGraph.graphConfig = cppGraph.graphConfig;

		// convert cppGraph to extendedGraph

		for (const auto& [_, cppNode] : cppGraph.nodeList) extendedGraph.nodeList.emplace_back(cppNodeToExtendedNode(cppNode));
		for (const auto& [_, cppComment] : cppGraph.commentList)
			extendedGraph.commentList.emplace_back(cppCommentToExtendedComment(cppComment));
		for (const auto& [_, cppPin] : cppGraph.pinList) extendedGraph.pinList.emplace_back(cppPinToExtendedPin(cppPin));

		extendedGraph.selectedNodeIDList = cppGraph.selectedNodeIdList;
		for (const auto& selectedNodeID : selectedNodeIDList) extendedGraph.selectedNodeIDList.push_back(selectedNodeID);
		extendedGraph.selectedCommentIDList = cppGraph.selectedCommentIdList;

		auto isMaterialOrSoundGraph = cppGraph.graphConfig.GetIsMaterialOrSoundGraph();

		// put pins in nodes
		for (const auto& [_, link] : cppGraph.linkList)
		{
			ExtendedPin& leftPin = extendedGraph.pinList[link.leftPinId];
			ExtendedPin& rightPin = extendedGraph.pinList[link.rightPinId];
			leftPin.connectedToPinIDList.push_back(rightPin.ID);
			rightPin.connectedToPinIDList.push_back(leftPin.ID);

			ExtendedNode& leftNode = extendedGraph.nodeList[leftPin.owningNodeID];
			const auto& leftCppPin = cppGraph.pinList.at(link.leftPinId);
			leftNode.touchPinIDList(leftCppPin.isLeft, isMaterialOrSoundGraph || leftCppPin.isExec).push_back(leftPin.ID);

			ExtendedNode& rightNode = extendedGraph.nodeList[rightPin.owningNodeID];
			const auto& rightCppPin = cppGraph.pinList.at(link.rightPinId);
			rightNode.touchPinIDList(rightCppPin.isLeft, isMaterialOrSoundGraph || rightCppPin.isExec).push_back(rightPin.ID);
		}

		// put nodes and comments in comments
		for (auto& [_, comment] : cppGraph.commentList)
		{
			for (const auto& [__, cppNode] : cppGraph.nodeList)
				if (comment.box.intersectWith(cppNode.box))
					extendedGraph.commentList[comment.id].insideNodeIDList.push_back(cppNode.id);
			for (const auto& [__, cppComment] : cppGraph.commentList)
				if (comment.id != cppComment.id && comment.box.intersectWith(cppComment.box))
					extendedGraph.commentList[comment.id].insideCommentIDList.push_back(cppComment.id);
		}
		// include or distinct
		for (auto& extendedComment : extendedGraph.commentList)
		{
			auto nodeIdSet = std::set<size_t>(extendedComment.insideNodeIDList.begin(), extendedComment.insideNodeIDList.end());
			for (auto& insideCommentID : std::vector(extendedComment.insideCommentIDList)) // browse copy
			{
				auto& insideExtendedComment = extendedGraph.commentList[insideCommentID];
				auto insideNodeIDSet = std::set<size_t>(
					insideExtendedComment.insideNodeIDList.begin(), insideExtendedComment.insideNodeIDList.end());
				auto intersectionNodeSet = intersection(nodeIdSet, insideNodeIDSet);
				if (intersectionNodeSet.empty() || insideNodeIDSet.size() > nodeIdSet.size())
				{
					// distinct or current is smaller (i.e potentially included)
					extendedComment.insideCommentIDList.erase(std::remove(extendedComment.insideCommentIDList.begin(),
																  extendedComment.insideCommentIDList.end(),
																  insideCommentID),
						extendedComment.insideCommentIDList.end());
					continue;
				}
				// include
				for (auto newNodeID : difference(insideNodeIDSet, nodeIdSet))
					extendedComment.insideNodeIDList.push_back(newNodeID);
			}
		}

		// reserve memory for the extendedGraph

		for (auto& node : extendedGraph.nodeList)
		{
			node.leftExecConnectionList.reserve(2 * node.leftExecConnectionList.size());
			node.rightExecConnectionList.reserve(2 * node.rightExecConnectionList.size());
			node.leftNonExecConnectionList.reserve(2 * node.leftNonExecConnectionList.size());
			node.rightNonExecConnectionList.reserve(2 * node.rightNonExecConnectionList.size());
		}
		for (auto& pin : extendedGraph.pinList) pin.connectedToPinIDList.reserve(2 * pin.connectedToPinIDList.size());

		extendedGraph.nodeList.reserve(2 * cppGraph.nodeList.size());
		extendedGraph.commentList.reserve(2 * cppGraph.commentList.size());
		extendedGraph.pinList.reserve(2 * cppGraph.pinList.size());
		extendedGraph.selectedNodeIDList.reserve(2 * cppGraph.selectedNodeIdList.size());
		extendedGraph.selectedCommentIDList.reserve(2 * cppGraph.selectedCommentIdList.size());

		for (auto& comment : extendedGraph.commentList)
		{
			comment.insideNodeIDList.reserve(2 * comment.insideNodeIDList.size());
			comment.insideCommentIDList.reserve(2 * comment.insideCommentIDList.size());
		}

		if (extendedGraph.selectedNodeIDList.empty())
		{
			for (const auto& [_, cppNode] : cppGraph.nodeList) extendedGraph.selectedNodeIDList.push_back(cppNode.id);
			extendedGraph.selectedCommentIDList = cppGraph.selectedCommentIdList;
			extendedGraph.selectedNodeIDList.reserve(2 * extendedGraph.selectedNodeIDList.size());
			extendedGraph.selectedCommentIDList.reserve(2 * extendedGraph.selectedCommentIDList.size());
		}

		extendedGraph.doAll();

		const GraphInstruction& graphInstruction = extendedGraph.instructionGlobalContainer;
		return graphInstruction;
	}

private:
	static inline ExtendedPin cppPinToExtendedPin(const CppPin& cppPin)
	{
		ExtendedPin extendedPin;
		extendedPin.ID = cppPin.id;
		extendedPin.name = cppPin.name;
		extendedPin.offset = cppPin.offset;
		extendedPin.owningNodeID = cppPin.ownerNodeId;
		return extendedPin;
	};

	static inline ExtendedNode cppNodeToExtendedNode(const CppNode& cppNode)
	{
		ExtendedNode extendedNode;
		extendedNode.ID = cppNode.id;
		extendedNode.name = cppNode.name;
		extendedNode.pos = cppNode.box.Min;
		extendedNode.size = cppNode.box.getSize();
		extendedNode.isRerouteNode = cppNode.isRerouteNode;
		return extendedNode;
	}

	static inline ExtendedComment cppCommentToExtendedComment(const CppComment& cppComment)
	{
		ExtendedComment extendedComment;
		extendedComment.ID = cppComment.id;
		extendedComment.name = cppComment.name;
		extendedComment.pos = cppComment.box.Min;
		extendedComment.size = cppComment.box.getSize();
		return extendedComment;
	}
};
