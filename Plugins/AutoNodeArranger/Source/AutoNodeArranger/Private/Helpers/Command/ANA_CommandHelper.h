// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

class ANA_CommandHelper
{
	ANA_CommandHelper() = delete;

public:
	static void ArrangeNodesStraight();
	static void ArrangeNodesCompact();
	static void ArrangeNodesCenter();
	static void SelectConnectedGraph();
	static void AddCustomConfig();
	static void ToggleUseArrangement();

	static void ArrangeNodes(bool bCompact, bool bCenterize);
};
