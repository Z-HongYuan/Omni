// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "ANA_GraphExporter.h"

#include "../../CppGraphArranger/DataTypes/Graph/CppGraph.h"
#include "../ANA_InputProcessor.h"
#include "./ANA_NodeRegister.h"
#include "./ANA_RerouteUtils.h"
#include "EdGraphNode_Comment.h"
#include "SGraphPanel.h"

bool ANA_GraphExporter::ExportGraph(const CppGraphConfig& graphConfig, ANA_GraphExport& graphExport, bool isArranging)
{
	auto isRerouteFunc = ANA_RerouteUtils::IsRerouteFunc(graphConfig.graphType == ECpp_GraphType::MATERIAL);
	bool isGraphMaterialOrSound
		= (graphConfig.graphType == ECpp_GraphType::MATERIAL || graphConfig.graphType == ECpp_GraphType::SOUND);

	auto graphPanel = graphExport.graphPanel;
	graphExport = ANA_GraphExport(graphExport.graphPanel); // full reset
	auto& graph = graphExport.cppGraph;
	auto& graphNodeList = graphExport.graphNodeList;
	auto& graphCommentList = graphExport.graphCommentList;
	auto& graphPinList = graphExport.graphPinList;

	graph.graphConfig = graphConfig;

	TMap<UEdGraphPin*, int> pinToPinIdMap;

	if (isArranging
		&& ANA_NodeRegister::Get().RegisterGraph(
			graphPanel, graphConfig.GetIsMaterialGraph(), graphConfig.bCompact, graphConfig.bCenterize))
		return false;

	for (auto& graphNode : graphPanel->GetGraphObj()->Nodes)
	{
		if (!IsValid(graphNode)) continue;
		std::string nodeName = std::string(TCHAR_TO_UTF8(*graphNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString()));
		auto pos = Vector2(graphNode->NodePosX, graphNode->NodePosY);
		if (auto comment = Cast<UEdGraphNode_Comment>(graphNode))
		{
			// Add comment
			auto commentId = graph.commentList.size();
			graphCommentList.Add(comment);
			if (graphPanel->SelectionManager.IsNodeSelected(graphNode)) graph.selectedCommentIdList.push_back(commentId);
			auto commentSize = Vector2(comment->NodeWidth, comment->NodeHeight);
			graph.commentList[commentId] = CppComment(commentId, nodeName, Box::PosSize(pos, commentSize));
		}
		else
		{
			// Add node
			auto nodeId = graph.nodeList.size();
			graphNodeList.Add(graphNode);
			if (graphPanel->SelectionManager.IsNodeSelected(graphNode)) graph.selectedNodeIdList.push_back(nodeId);
			bool isRerouteNode = isRerouteFunc(graphNode);
			auto nodeSize = ANA_NodeRegister::Get().GetNodeSize(graphPanel, graphNode);
			auto box = Box::PosSize(pos, nodeSize);
			graph.nodeList[nodeId] = CppNode(nodeId, nodeName, box, isRerouteNode);

			for (auto pin : graphNode->Pins)
			{
				// Add pin
				if (pin->WasTrashed() || pin->LinkedTo.Num() == 0) continue;
				size_t pinId = graph.pinList.size();
				std::string pinName = std::string(TCHAR_TO_UTF8(*pin->GetDisplayName().ToString()));
				bool bLeft = isGraphMaterialOrSound != (pin->Direction == EGPD_Input);
				bool bExec = pin->PinType.PinCategory == TEXT("exec");
				Vector2 pinOffset = ANA_NodeRegister::Get().GetPinOffset(graphPanel, graphNode, pin);
				graph.pinList[pinId] = CppPin(pinId,
					nodeId,
					pinName,
					bExec,
					bLeft,
					isRerouteNode ? (bLeft ? -REROUTE_PIN_OFFSET : REROUTE_PIN_OFFSET)
								  : Vector2(bLeft ? 16.0 : nodeSize.x - 16.0, pinOffset.y));
				graphPinList.Add(pin);
				pinToPinIdMap.Add(pin, pinId);
			}
		}
	}

	// set all links
	for (auto& [graphPin, cppPinId] : pinToPinIdMap)
	{
		if (graphPin->Direction == EGPD_Input) continue;
		for (auto otherGraphPin : graphPin->LinkedTo)
			if (int* otherPinID = pinToPinIdMap.Find(otherGraphPin))
			{
				size_t linkId = graph.linkList.size();
				graph.linkList[linkId] = CppLink(linkId, cppPinId, *otherPinID);
			}
	}

	return true;
}
