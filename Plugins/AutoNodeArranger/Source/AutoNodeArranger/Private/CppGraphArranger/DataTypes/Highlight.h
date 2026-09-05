// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include <string>

#include "../../env.h"

#include "../DataTypes/Graph/CppGraph.h"
#include "./Instruction/DeleteInstruction.h"
#include "./Instruction/LinkInstruction.h"
#include "./Instruction/NodeInstruction.h"
#include "./Instruction/RerouteInstruction.h"

enum class HighlightType
{
	Node,
	Comment,
	Pin,
	Link,
	Graph,
};

static std::string HighlightTypeToString(HighlightType type)
{
	switch (type)
	{
	case HighlightType::Node:
		return "node";
	case HighlightType::Comment:
		return "comment";
	case HighlightType::Pin:
		return "pin";
	case HighlightType::Link:
		return "link";
	case HighlightType::Graph:
		return "graph";
	}
	return "Unknown";
}

struct Highlight
{
	Highlight() = default;

	Highlight(const std::string& action_,
		const std::string& category_,
		HighlightType type_,
		bool clear_,
		bool active_ = false,
		size_t id_ = -1) :
		action(action_), category(category_), type(HighlightTypeToString(type_)), clear(clear_), active(active_), id(id_)
	{
	}

	Highlight(const std::string& action_,
		const std::string& category_,
		const RerouteInstruction& rerouteInstruction_,
		size_t id_ = -1) :
		action(action_), category(category_), type("reroute_ins"), id(id_), rerouteInstruction(rerouteInstruction_)
	{
	}

	Highlight(
		const std::string& action_, const std::string& category_, const LinkInstruction& linkInstruction_, size_t id_ = -1) :
		action(action_), category(category_), type("link_ins"), id(id_), linkInstruction(linkInstruction_)
	{
	}

	Highlight(
		const std::string& action_, const std::string& category_, const DeleteInstruction& deleteInstruction_, size_t id_ = -1) :
		action(action_), category(category_), type("delete_ins"), id(id_), deleteInstruction(deleteInstruction_)
	{
	}

	Highlight(
		const std::string& action_, const std::string& category_, const NodeInstruction& nodeInstruction_, size_t id_ = -1) :
		action(action_), category(category_), type("node_ins"), id(id_), nodeInstruction(nodeInstruction_)
	{
	}

	explicit Highlight(const CppGraph& graph_) : action("Graph"), category("Set graph"), type("graph"), id(-1), graph(graph_) {}

	std::string action;
	std::string category;
	std::string type;
	bool clear = false;
	bool active = false;
	size_t id = static_cast<size_t>(-1);

	RerouteInstruction rerouteInstruction;
	LinkInstruction linkInstruction;
	DeleteInstruction deleteInstruction;
	NodeInstruction nodeInstruction;

	CppGraph graph;
};

#include <vector>

static std::vector<Highlight> globalHiglights;

#ifdef DEBUG_HIGHLIGHT
#define HIGHLIGHT(...) globalHiglights.push_back(Highlight(__FUNCTION__, __VA_ARGS__))
#else
#define HIGHLIGHT(...)
#endif
