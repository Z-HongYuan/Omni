// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "../ANA_CacheHelper.h"
#include "../DataTypes/ANA_EventDispatcher.h"
#include "../DataTypes/Singleton.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "SGraphPanel.h"

class ANA_NodeRegister : public Singleton<ANA_NodeRegister>
{
public:
	Vector2 GetNodeSize(const SGraphPanel* graphPanel, const UEdGraphNode* node);

	Vector2 GetPinOffset(const SGraphPanel* graphPanel, const UEdGraphNode* node, const UEdGraphPin* pin);

	// returns true if some nodes to register
	// compact and centerize parameters used for arranging when registration needed
	bool RegisterGraph(SGraphPanel* graphPanel, bool bMaterialGraph, bool bCompact, bool bCenterize);

	void StartAutoRegister();

	void StopAutoRegister();

private:
	TArray<UEdGraphNode*> GetNodeToRegisterList(const SGraphPanel* graphPanel, bool bMaterialGraph);

	TMap<FGuid, ANA_GraphData> graphDataMap;
	ANA_EventID autoRegisterTicksId;

	bool isRegisteringGraph = false;
};
