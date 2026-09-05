// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "../../Helpers/DataTypes/ANA_CacheData.h"
#include "../DataTypes/Graph/CppGraph.h"
#include "../DataTypes/Highlight.h"
#include "../DataTypes/TestGraphResult.h"
#include "../Libs/bsttJson/bsttJson.hpp"

// named ToJson with uppercase in order to highlight the unusual transformation from map to vector
template <typename T> inline Json ToJson(const std::map<size_t, T>& m)
{
	JsonArr jsonArr;
	for (const auto& [_, t] : m) jsonArr.emplace_back(toJson(t));
	return jsonArr;
}

// transform vector<T> to map<size_t, T>, requires T to have id field
template <typename T> std::map<size_t, T> ToMap(const std::vector<T>& vec)
{
	std::map<size_t, T> m;
	for (const auto& t : vec) m[t.id] = t;
	return m;
}

template <> inline Json toJson<size_t>(const size_t& i) { return Json(i); }
template <> inline size_t fromJson<size_t>(const Json& json) { return static_cast<size_t>(json); }

template <> inline Json toJson<Vector2>(const Vector2& vec) { return JsonObj{{"x", vec.x}, {"y", vec.y}}; }
template <> inline Vector2 fromJson<Vector2>(const Json& json) { return Vector2(json["x"], json["y"]); }

template <> inline Json toJson<Box>(const Box& box) { return JsonObj{{"Min", box.Min}, {"Max", box.Max}}; }
template <> inline Box fromJson<Box>(const Json& json) { return Box(json["Min"], json["Max"]); }

template <> inline Json toJson<CppPin>(const CppPin& pin)
{
	return JsonObj{{"id", pin.id},
		{"ownerNodeId", pin.ownerNodeId},
		{"name", pin.name},
		{"isExec", pin.isExec},
		{"isLeft", pin.isLeft},
		{"offset", pin.offset}};
}
template <> inline CppPin fromJson<CppPin>(const Json& json)
{
	return CppPin(json["id"], json["ownerNodeId"], json["name"], json["isExec"], json["isLeft"], json["offset"]);
}

template <> inline Json toJson<CppNode>(const CppNode& node)
{
	return JsonObj{{"id", node.id}, {"name", node.name}, {"box", node.box}, {"isRerouteNode", node.isRerouteNode}};
}
template <> inline CppNode fromJson<CppNode>(const Json& json)
{
	return CppNode(json["id"], json["name"], json["box"], json["isRerouteNode"]);
}

template <> inline Json toJson<CppLink>(const CppLink& link)
{
	return JsonObj{{"id", link.id}, {"leftPinId", link.leftPinId}, {"rightPinId", link.rightPinId}};
}
template <> inline CppLink fromJson<CppLink>(const Json& json)
{
	return CppLink(json["id"], json["leftPinId"], json["rightPinId"]);
}

template <> inline Json toJson<CppComment>(const CppComment& comment)
{
	return JsonObj{{"id", comment.id}, {"name", comment.name}, {"box", comment.box}};
}
template <> inline CppComment fromJson<CppComment>(const Json& json) { return CppComment(json["id"], json["name"], json["box"]); }

template <> inline ECpp_ArrangeSelectionType fromJson<ECpp_ArrangeSelectionType>(const Json& json)
{
	int i = json;
	return static_cast<ECpp_ArrangeSelectionType>(i);
}

template <> inline ECpp_GraphType fromJson<ECpp_GraphType>(const Json& json)
{
	int i = json;
	return static_cast<ECpp_GraphType>(i);
}


template <> inline Json toJson<CppGraphConfig>(const CppGraphConfig& graphConfig)
{
	return JsonObj{{"execSpacingX", graphConfig.execSpacingX},
		{"spacing", graphConfig.spacing},
		{"commentSpacing", graphConfig.commentSpacing},
		{"bGroupAllConnectedGraph", graphConfig.bGroupAllConnectedGraph},
		// {"bUseArrangement", graphConfig.bUseArrangement}, // value not used
		{"arrangeSelectionType", static_cast<int>(graphConfig.arrangeSelectionType)},
		{"lineWidth", graphConfig.lineWidth},
		{"bAutoGenerateReroute", graphConfig.bAutoGenerateReroute},
		{"bCompact", graphConfig.bCompact},
		{"bCenterize", graphConfig.bCenterize},
		{"graphType", static_cast<int>(graphConfig.graphType)},
		{"bUseReroutePlacerY", graphConfig.bUseReroutePlacerY}};
}
template <> inline CppGraphConfig fromJson<CppGraphConfig>(const Json& json)
{
	return CppGraphConfig(json["execSpacingX"],
		json["spacing"],
		json["commentSpacing"],
		true, // always true on read or write
		static_cast<ECpp_ArrangeSelectionType>(json["arrangeSelectionType"]),
		json["bGroupAllConnectedGraph"],
		json["lineWidth"],
		json["bAutoGenerateReroute"],
		json["bCompact"],
		json["bCenterize"],
		static_cast<ECpp_GraphType>(json["graphType"]),
		json["bUseReroutePlacerY"]);
}

template <> inline Json toJson<CppGraph>(const CppGraph& graph)
{
	return JsonObj{{"graphConfig", graph.graphConfig},
		{"nodeList", ToJson(graph.nodeList)},
		{"commentList", ToJson(graph.commentList)},
		{"pinList", ToJson(graph.pinList)},
		{"linkList", ToJson(graph.linkList)},
		{"selectedNodeIdList", graph.selectedNodeIdList},
		{"selectedCommentIdList", graph.selectedCommentIdList}};
}
template <> inline CppGraph fromJson<CppGraph>(const Json& json)
{
	CppGraph cppGraph;
	json.get("graphConfig", cppGraph.graphConfig);
	std::vector<CppNode> nodeList;
	json.get("nodeList", nodeList);
	cppGraph.nodeList = ToMap(nodeList);
	std::vector<CppComment> commentList;
	json.get("commentList", commentList);
	cppGraph.commentList = ToMap(commentList);
	std::vector<CppPin> pinList;
	json.get("pinList", pinList);
	cppGraph.pinList = ToMap(pinList);
	std::vector<CppLink> linkList;
	json.get("linkList", linkList);
	cppGraph.linkList = ToMap(linkList);
	json.get("selectedNodeIdList", cppGraph.selectedNodeIdList);
	json.get("selectedCommentIdList", cppGraph.selectedCommentIdList);
	return cppGraph;
}

template <> inline Json toJson<RerouteInstruction>(const RerouteInstruction& rerouteInstruction)
{
	return JsonObj{{"pos", rerouteInstruction.pos}};
}
template <> inline RerouteInstruction fromJson<RerouteInstruction>(const Json& json)
{
	return RerouteInstruction(Vector2(json["pos"]));
}

template <> inline Json toJson<LinkInstruction>(const LinkInstruction& linkInstruction)
{
	return JsonObj{
		{"fromPinId", linkInstruction.fromPinId}, {"toPinId", linkInstruction.toPinId}, {"bBreak", linkInstruction.bBreak}};
}
template <> inline LinkInstruction fromJson<LinkInstruction>(const Json& json)
{
	return LinkInstruction(json["fromPinId"], json["toPinId"], json["bBreak"]);
}

template <> inline Json toJson<DeleteInstruction>(const DeleteInstruction& deleteInstruction)
{
	return JsonObj{{"nodeId", deleteInstruction.nodeId}, {"bCancel", deleteInstruction.bCancel}};
}
template <> inline DeleteInstruction fromJson<DeleteInstruction>(const Json& json)
{
	return DeleteInstruction(size_t(json["nodeId"]), json["bCancel"]);
}

template <> inline Json toJson<NodeInstruction>(const NodeInstruction& nodeInstruction)
{
	return JsonObj{{"nodeId", nodeInstruction.nodeId}, {"fromPos", nodeInstruction.fromPos}, {"toPos", nodeInstruction.toPos}};
}
template <> inline NodeInstruction fromJson<NodeInstruction>(const Json& json)
{
	return NodeInstruction(json["nodeId"], json["fromPos"], json["toPos"]);
}

template <> inline Json toJson<Highlight>(const Highlight& highlight)
{
	Json json = JsonObj{{"action", highlight.action}, {"category", highlight.category}};
	if (highlight.type == "reroute_ins") json["reroute"] = highlight.rerouteInstruction;
	else if (highlight.type == "link_ins")
		json["link"] = highlight.linkInstruction;
	else if (highlight.type == "delete_ins")
		json["delete"] = highlight.deleteInstruction;
	else if (highlight.type == "node_ins")
		json["node"] = highlight.nodeInstruction;
	else if (highlight.type == "graph")
		json["graph"] = highlight.graph;
	else
	{
		json["type"] = highlight.type;
		json["clear"] = highlight.clear;
		json["active"] = highlight.active;
	}
	if (highlight.id != static_cast<size_t>(-1)) json["id"] = highlight.id;
	return json;
}
template <> inline Highlight fromJson<Highlight>(const Json& json)
{
	Highlight highlight;
	json.get("action", highlight.action, "category", highlight.category);
	if (json.tryGet("reroute", highlight.rerouteInstruction)) highlight.type = "reroute_ins";
	else if (json.tryGet("link", highlight.linkInstruction))
		highlight.type = "link_ins";
	else if (json.tryGet("delete", highlight.deleteInstruction))
		highlight.type = "delete_ins";
	else if (json.tryGet("node", highlight.nodeInstruction))
		highlight.type = "node_ins";
	else if (json.tryGet("graph", highlight.graph))
		highlight.type = "graph";
	else
	{
		json.get("type", highlight.type);
		json.get("clear", highlight.clear);
		json.get("active", highlight.active);
	}
	json.tryGet("id", highlight.id);
	return highlight;
}

template <> inline Json toJson<TestGraphResult>(const TestGraphResult& testGraphResult)
{
	return JsonObj{{"name", testGraphResult.name},
		{"location", testGraphResult.location},
		{"seed", std::to_string(testGraphResult.seed)},
		{"initialGraph", testGraphResult.initialGraph},
		{"highlights", testGraphResult.highlights}};
}

template <> inline Json toJson<ANA_ReleaseNotifierData>(const ANA_ReleaseNotifierData& releaseNotifierData)
{
	return JsonObj{{"version", releaseNotifierData.version}};
}
template <> inline ANA_ReleaseNotifierData fromJson<ANA_ReleaseNotifierData>(const Json& json)
{
	ANA_ReleaseNotifierData releaseNotifierData;
	json.tryGet("version", releaseNotifierData.version);
	return releaseNotifierData;
}

template <> inline Json toJson<ANA_NodeData>(const ANA_NodeData& nodeData)
{
	return JsonObj{{"size", nodeData.size}, {"pinOffsetMap", nodeData.pinOffsetMap}};
}
template <> inline ANA_NodeData fromJson<ANA_NodeData>(const Json& json)
{
	ANA_NodeData nodeData;
	json.tryGet("size", nodeData.size, "pinOffsetMap", nodeData.pinOffsetMap);
	return nodeData;
}

template <> inline Json toJson<ANA_GraphData>(const ANA_GraphData& graphData)
{
	return JsonObj{{"nodeDataMap", graphData.nodeDataMap}};
}
template <> inline ANA_GraphData fromJson<ANA_GraphData>(const Json& json)
{
	ANA_GraphData graphData;
	json.tryGet("nodeDataMap", graphData.nodeDataMap);
	return graphData;
}
