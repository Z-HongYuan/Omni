// Copyright 2024 bstt, Inc. All Rights Reserved.

#include "main.h"

#include "../env.h"

#include <fstream>

#include "./CppGraphArranger.h"
#include "./DataTypes/Graph/CppGraph.h"
#include "./DataTypes/Highlight.h"
#include "./Helpers/AllJsonConverters.h"
#include "./Helpers/GraphBuilder.h"

// #include "../OldCppGraphArranger/OldCppGraphArranger.h"

GraphInstruction mainFn(const CppGraph& cppGraph)
{
	// GraphInstruction graphInstruction = OldCppGraphArranger::ArrangeGraph(cppGraph);
	GraphInstruction graphInstruction = CppGraphArranger::ArrangeGraph(cppGraph);
#ifdef OUTPUT_FILE_PATH
	std::ofstream file(OUTPUT_FILE_PATH);
	Json json = JsonObj{{"cppGraph", cppGraph}, {"highlights", globalHiglights}};
	file << json << std::endl;
#endif
	return graphInstruction;
}

static CppGraphConfig DEFAULT_GRAPH_CONFIG(
	120.0, Vector2(50.0, 30.0), Vector2(24.0, 24.0), true, ECpp_ArrangeSelectionType::OneForAll, true, 5.0, true, false, false, ECpp_GraphType::BLUEPRINT, false);

static CppGraph readGraph()
{
	CppGraph cppGraph;
#ifdef OUTPUT_FILE_PATH
	Json json = Json::parseFile(OUTPUT_FILE_PATH);
	json.get("cppGraph", cppGraph);
#endif
	return cppGraph;
}

bool b_use_file = true;

int main(int argc, char** argv)
{
	std::cout << "start" << std::endl;
	int64_t seed = timeSeed();
	CppGraph cppGraph;

	if (b_use_file) cppGraph = readGraph();
	else
	{
		if (argc == 2) seed = atoll(argv[1]); // seed from command line

		GraphBuilder builder(seed, DEFAULT_GRAPH_CONFIG);

		// ======= START =======

		auto c0 = builder.addComment();
		auto nodeA = builder.addNode();
		auto nodeB = nodeA.linkRight();
		auto nodeC = nodeB.linkRight().placeBelow(nodeA).comment(c0);
		auto nodeD = nodeC.linkLeft().placeBelow(nodeA).comment(c0);
		nodeD.linkLeft().placeBelow(nodeA).comment(c0);
		auto nodeF = builder.addNode().placeBelow(c0).comment(c0);
		nodeB.placeBelow(c0);
		auto nodeG = nodeA.linkRight().placeBelow(nodeB);
		nodeG.linkRight().placeBelow(nodeB).linkRight().placeBelow(nodeB).linkRight(nodeC);
		nodeG.linkRight().placeBelow(nodeG).linkRight(nodeF);
		builder.exec();

		cppGraph = builder.getGraph();

		// ======= END =======
	}

	try
	{
		CppGraphArranger::ArrangeGraph(cppGraph); // graph instruction already put in highlights
	}
	catch (const std::exception& err)
	{
		std::cerr << "Error: " << err.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << "Unknown error" << std::endl;
	}

	std::cout << "seed: " << seed << std::endl;

#ifdef OUTPUT_FILE_PATH
	std::ofstream file(OUTPUT_FILE_PATH);
	Json json = JsonObj{{"seed", std::to_string(seed)}, {"cppGraph", cppGraph}, {"highlights", globalHiglights}};
	file << json << std::endl;
#endif

	return 0;
}
