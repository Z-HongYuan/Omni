// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <map>
#include <string>
#include <vector>

#include "../DataTypes/Highlight.h"
#include "./Graph/CppGraph.h"

struct TestGraphResult
{
	TestGraphResult() = default;

	TestGraphResult(const std::string& name_,
		const std::string& location_,
		size_t seed_,
		const CppGraph& initialGraph_,
		const std::vector<Highlight>& highlights_) :
		name(name_), location(location_), seed(seed_), initialGraph(initialGraph_), highlights(highlights_)
	{
	}

	std::string name;
	std::string location;
	size_t seed;

	CppGraph initialGraph;
	std::vector<Highlight> highlights;
};

#include <vector>

static std::map<std::string, std::vector<TestGraphResult>> globalTestResultListMap;
