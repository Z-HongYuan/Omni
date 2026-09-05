// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "ANA_InstructionsExecutor.h"

#include "../../CppGraphArranger/Helpers/ExtendedCommon.h"
#include "../DataTypes/ANA_Constants.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphSchema_K2_Actions.h"
#include "K2Node_Knot.h"
#include "SGraphNodeComment.h"

void ANA_InstructionsExecutor::ExecuteGraphInstructionWithoutNode(
	ANA_GraphExport& graphExport, const GraphInstruction& instruction)
{
	// executeRerouteInstruction
	auto graphObj = graphExport.graphPanel->GetGraphObj();
	graphObj->Modify();
	for (const auto& rerouteInstruction : instruction.rerouteInstructionList)
	{
		UK2Node_Knot* rerouteNode = FEdGraphSchemaAction_K2NewNode::SpawnNode<UK2Node_Knot>(
			graphObj, FVector2D(rerouteInstruction.pos.x, rerouteInstruction.pos.y), EK2NewNodeFlags::None);
		size_t nodeId = graphExport.cppGraph.nodeList.size();
		graphExport.cppGraph.nodeList[nodeId] = CppNode(nodeId,
			"reroute_" + std::to_string(nodeId),
			Box::PosSize(rerouteInstruction.pos, Vector2(rerouteNode->NodeWidth, rerouteNode->NodeHeight)),
			true);
		graphExport.graphNodeList.Add(rerouteNode);
		graphExport.graphPinList.Add(rerouteNode->GetInputPin());
		graphExport.graphPinList.Add(rerouteNode->GetOutputPin());
	}

	// executeLinkInstruction
	for (const auto& linkInstruction : instruction.linkInstructionList)
	{
		auto fromPin = graphExport.graphPinList[linkInstruction.fromPinId];
		auto toPin = graphExport.graphPinList[linkInstruction.toPinId];
		fromPin->Modify();
		toPin->Modify();
		if (linkInstruction.bBreak) fromPin->BreakLinkTo(toPin);
		else
			fromPin->MakeLinkTo(toPin);
		UEdGraphNode* fromNode = fromPin->GetOwningNodeUnchecked();
		if (fromNode) fromNode->PinConnectionListChanged(fromPin);
		UEdGraphNode* toNode = toPin->GetOwningNodeUnchecked();
		if (toNode) toNode->PinConnectionListChanged(toPin);
	}

	// executeCommentInstruction
	for (const auto& commentInstruction : instruction.commentInstructionList)
	{
		auto comment = graphExport.graphCommentList[commentInstruction.commentId];
		comment->Modify();
		// executeCommentInstructionForNode
		for (const auto& nodeIdUnder : commentInstruction.nodeIdUnderCommentList)
		{
			auto node = graphExport.graphNodeList[nodeIdUnder];
			comment->AddNodeUnderComment(node);
		}
		// executeCommentInstructionForComment
		for (const auto& commentIdUnder : commentInstruction.commentIdUnderCommentList)
		{
			auto commentUnder = graphExport.graphCommentList[commentIdUnder];
			comment->AddNodeUnderComment(commentUnder);
		}
	}

	// executeDeleteInstruction
	for (const auto& deleteInstruction : instruction.deleteInstructionList)
	{
		auto node = graphExport.graphNodeList[deleteInstruction.nodeId];
		node->Modify();
		node->DestroyNode();
	}
}

void ANA_InstructionsExecutor::ExecuteNodeInstructions(
	const ANA_GraphExport& graphExport, const GraphInstruction& instruction, double alpha)
{
	// move nodes
	for (auto& nodeInstruction : instruction.mergedNodeInstructionList)
	{
		auto node = graphExport.graphNodeList[nodeInstruction.nodeId];
		node->Modify();
		node->NodePosX = nodeInstruction.fromPos.x + (nodeInstruction.toPos.x - nodeInstruction.fromPos.x) * alpha;
		node->NodePosY = nodeInstruction.fromPos.y + (nodeInstruction.toPos.y - nodeInstruction.fromPos.y) * alpha;
	}

	if (ANA_Constants::isAutoSizeCommentLoaded) return;

	// move comments
	for (const auto& commentInstruction : instruction.commentInstructionList)
	{
		if (commentInstruction.nodeIdUnderCommentList.empty()) continue;

		auto comment = graphExport.graphCommentList[commentInstruction.commentId];
		comment->Modify();
		auto firstGraphNode = graphExport.graphNodeList[commentInstruction.nodeIdUnderCommentList.front()];
		Box commentBox = Box::PosSize(Vector2(firstGraphNode->NodePosX, firstGraphNode->NodePosY),
			Vector2(firstGraphNode->NodeWidth, firstGraphNode->NodeHeight));
		for (const auto& nodeIdUnder : commentInstruction.nodeIdUnderCommentList)
		{
			auto graphNode = graphExport.graphNodeList[nodeIdUnder];
			auto& cppNode = graphExport.cppGraph.nodeList.at(nodeIdUnder); // use size stored in cppGraph
			commentBox += Box::PosSize(Vector2(graphNode->NodePosX, graphNode->NodePosY), cppNode.box.getSize());
		}
		for (const auto& commentIdUnder : commentInstruction.commentIdUnderCommentList)
		{
			auto graphComment = graphExport.graphCommentList[commentIdUnder];
			commentBox += Box::PosSize(Vector2(graphComment->NodePosX, graphComment->NodePosY),
				Vector2(graphComment->NodeWidth, graphComment->NodeHeight));
		}
		commentBox = commentBox.extendBox(graphExport.cppGraph.graphConfig.commentSpacing).withCommentHeader();
		auto commentBoxSize = commentBox.getSize();
		comment->NodePosX = commentBox.Min.x;
		comment->NodePosY = commentBox.Min.y;
		comment->NodeWidth = commentBoxSize.x;
		comment->NodeHeight = commentBoxSize.y;

		auto graphPanel = graphExport.graphPanel;
		TSharedPtr<SGraphNode> GraphNode = graphPanel->GetNodeWidgetFromGuid(comment->NodeGuid);
		if (GraphNode.IsValid())
		{
			auto GraphNodeComment = StaticCastSharedPtr<SGraphNodeComment>(GraphNode);
			if (GraphNodeComment)
			{
				SGraphNodeComment::FArguments args;
				GraphNodeComment->Construct(args, comment);
			}
		}
	}
}

void ANA_InstructionsExecutor::ExecuteSelectInstruction(const ANA_GraphExport& graphExport, const SelectInstruction& instruction)
{
	FGraphPanelSelectionSet selection;
	for (auto& nodeId : instruction.nodeIdSet) selection.Add(graphExport.graphNodeList[nodeId]);
	for (auto& commentId : instruction.commentIdSet) selection.Add(graphExport.graphCommentList[commentId]);

	graphExport.graphPanel->SelectionManager.SetSelectionSet(selection);
}
