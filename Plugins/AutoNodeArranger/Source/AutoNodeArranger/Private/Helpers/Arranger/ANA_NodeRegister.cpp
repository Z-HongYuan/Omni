// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "ANA_NodeRegister.h"

#include "../../Helpers/Command/ANA_CommandHelper.h"
#include "../../Helpers/Command/ANA_GraphHelper.h"
#include "../ANA_InputProcessor.h"
#include "./ANA_RerouteUtils.h"

#include <cmath>
#include <thread>

// zoom amount from which registration no longer necessary
static const float VALID_ZOOM_AMOUNT = 0.67f; // value for zoom -3

static const FVector2D FCOMMENT_HEADER(0.f, 30.f);

template <typename T, typename U> const U* Find(const std::map<T, U>& m, const T& key)
{
	auto it = m.find(key);
	return it == m.end() ? nullptr : &it->second;
}

template <typename T, typename U> U& FindOrAdd(std::map<T, U>& m, const T& key) { return m[key]; }

std::string GuidToStdString(const FGuid& guid)
{
	FString f = guid.ToString();
	return TCHAR_TO_UTF8(*f);
}

Vector2 FVectorToVector(const FVector2D& v) { return Vector2(v.X, v.Y); }

Vector2 ANA_NodeRegister::GetNodeSize(const SGraphPanel* graphPanel, const UEdGraphNode* node)
{
	auto graphObj = graphPanel->GetGraphObj();
	auto graphPanelGuid = graphObj->GraphGuid;

	if (!graphDataMap.Contains(graphPanelGuid)) graphDataMap.Add(graphPanelGuid, ANA_CacheHelper::Get().GetGraphData(graphPanel));
	auto& graphData = graphDataMap[graphPanelGuid];

	// try get stored size
	if (auto nodeData = Find(graphData.nodeDataMap, GuidToStdString(node->NodeGuid))) return nodeData->size;

	// else compute size
	if (auto comment = Cast<UEdGraphNode_Comment>(node))
		return Vector2(comment->NodeWidth, comment->NodeHeight - FCOMMENT_HEADER.Y);
	if (auto sNode = graphPanel->GetNodeWidgetFromGuid(node->NodeGuid)) return FVectorToVector(sNode->ComputeDesiredSize(1.f));
	return Vector2();
}

Vector2 ANA_NodeRegister::GetPinOffset(const SGraphPanel* graphPanel, const UEdGraphNode* node, const UEdGraphPin* pin)
{
	auto graphObj = graphPanel->GetGraphObj();
	auto graphPanelGuid = graphObj->GraphGuid;

	if (!graphDataMap.Contains(graphPanelGuid)) graphDataMap.Add(graphPanelGuid, ANA_CacheHelper::Get().GetGraphData(graphPanel));
	auto& graphData = graphDataMap[graphPanelGuid];

	// try get store offset
	if (auto nodeData = Find(graphData.nodeDataMap, GuidToStdString(node->NodeGuid)))
		if (auto pinData = Find(nodeData->pinOffsetMap, GuidToStdString(pin->PinId))) return *pinData;

	// else retrieve offset
	if (auto sNode = graphPanel->GetNodeWidgetFromGuid(node->NodeGuid))
		if (auto sPin = sNode->FindWidgetForPin(const_cast<UEdGraphPin*>(pin))) return FVectorToVector(sPin->GetNodeOffset());
	return Vector2();
}

bool ANA_NodeRegister::RegisterGraph(SGraphPanel* graphPanel, bool bMaterialGraph, bool bCompact, bool bCenterize)
{
	TArray<UEdGraphNode*> graphNodeToRegisterList = GetNodeToRegisterList(graphPanel, bMaterialGraph);

	if (graphNodeToRegisterList.IsEmpty()) return false;

	isRegisteringGraph = true;

	FVector2D viewOffset = graphPanel->GetViewOffset();
	float zoomAmount = graphPanel->GetZoomAmount();
	FGuid bookmarkID = graphPanel->GetViewBookmarkId();
	int32 currentNodeIndex = graphNodeToRegisterList.Num() - 1;

	ANA_InputProcessor::tickEventId++;
	ANA_InputProcessor::Get().tickDispatcher.Add(ANA_InputProcessor::tickEventId,
		[this,
			tickEventId_ = ANA_InputProcessor::tickEventId,
			graphNodeToRegisterList,
			viewOffset,
			zoomAmount,
			bookmarkID,
			currentNodeIndex,
			graphPanel,
			bCompact,
			bCenterize](float deltaTime) mutable
		{
			if (currentNodeIndex < 0)
			{
				// restore to initial view
				graphPanel->RestoreViewSettings(viewOffset, zoomAmount, bookmarkID);
				isRegisteringGraph = false;
				ANA_InputProcessor::Get().tickDispatcher.Remove(tickEventId_);
				ANA_CommandHelper::ArrangeNodes(bCompact, bCenterize);
				return;
			}

			auto currentNode = graphNodeToRegisterList[currentNodeIndex];
			FVector2D nodePos = FVector2D(currentNode->NodePosX, currentNode->NodePosY);

			// move view to target node
			graphPanel->RestoreViewSettings(nodePos - graphPanel->GetCachedGeometry().GetLocalSize() * 0.5f, 1.f, bookmarkID);

			currentNodeIndex--;
		});

	return true;
}

void ANA_NodeRegister::StartAutoRegister()
{
	ANA_InputProcessor::Get().tickEventId++;
	autoRegisterTicksId = ANA_InputProcessor::Get().tickEventId;
	ANA_InputProcessor::Get().tickDispatcher.Add(autoRegisterTicksId,
		[this, autoRegisterCooldown = 0.f, setGraphDataCooldown = 0.f](const float DeltaTime) mutable
		{
			if (!isRegisteringGraph) // always auto registering when regestiring graph
			{
				autoRegisterCooldown -= DeltaTime;
				if (autoRegisterCooldown > 0.f) return;
				autoRegisterCooldown += 1.f; // auto registering nodes each second
			}
			auto graphPanel = ANA_GraphHelper::GetCurrentGraphPanel();
			if (!graphPanel) return;

			auto graphObj = graphPanel->GetGraphObj();
			auto graphPanelGuid = graphObj->GraphGuid;

			if (!graphDataMap.Contains(graphPanelGuid))
				graphDataMap.Add(graphPanelGuid, ANA_CacheHelper::Get().GetGraphData(graphPanel.Get()));
			auto& graphData = graphDataMap[graphPanelGuid];

			float zoomAmount = std::min(1.f, graphPanel->GetZoomAmount());
			if (zoomAmount < VALID_ZOOM_AMOUNT) return;

			for (auto node : graphObj->Nodes)
			{
				auto& nodeData = FindOrAdd(graphData.nodeDataMap, GuidToStdString(node->NodeGuid));

				FVector2D pos = FVector2D(node->NodePosX, node->NodePosY);
				if (!graphPanel->IsRectVisible(pos, pos)) continue;

				// get nodeData
				auto sNode = graphPanel->GetNodeWidgetFromGuid(node->NodeGuid);
				if (auto comment = Cast<UEdGraphNode_Comment>(node))
					nodeData.size = Vector2(comment->NodeWidth, comment->NodeHeight - FCOMMENT_HEADER.Y);
				else
					nodeData.size = sNode ? FVectorToVector(sNode->ComputeDesiredSize(1.f)) : Vector2();
				if (!sNode) continue;

				// get pinOffsetMap
				for (auto pin : node->Pins)
				{
					auto sPin = sNode->FindWidgetForPin(const_cast<UEdGraphPin*>(pin));
					if (!sPin) continue;
					nodeData.pinOffsetMap[GuidToStdString(pin->PinId)] = FVectorToVector(sPin->GetNodeOffset());
				}
			}

			setGraphDataCooldown -= DeltaTime;
			if (setGraphDataCooldown > 0.f) return;
			setGraphDataCooldown += 60.f; // set graph data each minutes

			// register all graphs
			for (auto& [guid, data] : graphDataMap) ANA_CacheHelper::Get().SetGraphData(guid, data);
			graphDataMap.Empty();
		});
}

void ANA_NodeRegister::StopAutoRegister()
{
	ANA_InputProcessor::Get().tickDispatcher.Remove(autoRegisterTicksId);
	// register all graphs
	for (auto& [guid, data] : graphDataMap) ANA_CacheHelper::Get().SetGraphData(guid, data);
	graphDataMap.Empty();
}

TArray<UEdGraphNode*> ANA_NodeRegister::GetNodeToRegisterList(const SGraphPanel* graphPanel, bool bMaterialGraph)
{
	TArray<UEdGraphNode*> result;
	auto& graphGuid = graphPanel->GetGraphObj()->GraphGuid;
	if (!graphDataMap.Contains(graphGuid)) graphDataMap.Add(graphGuid, ANA_CacheHelper::Get().GetGraphData(graphPanel));
	auto& graphData = graphDataMap[graphGuid];
	auto isRerouteFunc = ANA_RerouteUtils::IsRerouteFunc(bMaterialGraph);
	for (auto& node : graphPanel->GetGraphObj()->Nodes)
	{
		if (isRerouteFunc(node)) continue;
		auto nodeData = Find(graphData.nodeDataMap, GuidToStdString(node->NodeGuid));
		if (!nodeData || (nodeData->size.x == 0. && nodeData->size.y == 0)) result.Add(node);
	}
	return result;
}
