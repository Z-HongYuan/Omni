// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "EdGraph/EdGraphNode.h"
#include "K2Node_Knot.h"
#include "MaterialGraphNode_Knot.h"

class ANA_RerouteUtils
{
private:
	ANA_RerouteUtils() = delete;

public:
	static inline auto IsRerouteFunc(bool bMaterial) { return bMaterial ? isMaterialReroute : isOtherReroute; }

private:
	static inline bool isMaterialReroute(const UEdGraphNode* node)
	{
		return node->GetClass()->IsChildOf<UMaterialGraphNode_Knot>();
	}
	static inline bool isOtherReroute(const UEdGraphNode* node) { return node->GetClass()->IsChildOf<UK2Node_Knot>(); }
};
