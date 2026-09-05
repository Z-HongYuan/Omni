// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "../../CppGraphArranger/DataTypes/Basic/Vector2.h"

#include <map>
#include <string>

struct ANA_ReleaseNotifierData
{
	ANA_ReleaseNotifierData() = default;
	ANA_ReleaseNotifierData(size_t version_) : version(version_) {}

	size_t version = 0;
};

struct ANA_NodeData
{
	ANA_NodeData() = default;
	ANA_NodeData(Vector2 size_) : size(size_) {}
	ANA_NodeData(Vector2 size_, const std::map<std::string, Vector2>& pinOffsetMap_) : size(size_), pinOffsetMap(pinOffsetMap_) {}

	Vector2 size;
	std::map<std::string, Vector2> pinOffsetMap; // key is Guid, value is pinOffset
};

struct ANA_GraphData
{
	ANA_GraphData() = default;
	ANA_GraphData(const std::map<std::string, ANA_NodeData>& nodeDataMap_) : nodeDataMap(nodeDataMap_) {}

	std::map<std::string, ANA_NodeData> nodeDataMap; // key is Guid, value is nodeData
};
