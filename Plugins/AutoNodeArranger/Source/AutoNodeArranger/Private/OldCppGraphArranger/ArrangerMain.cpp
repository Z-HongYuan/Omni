// Copyright 2024 bstt, Inc. All Rights Reserved.

#include "ArrangerMain.h"

#include <fstream>

#include "../CppGraphArranger/Helpers/AllJsonConverters.h"
#include "./OldCppGraphArranger.h"

GraphInstruction oldMainFn(const CppGraph& cppGraph)
{
	GraphInstruction graphInstruction = OldCppGraphArranger::ArrangeGraph(cppGraph);
#ifdef OUTPUT_FILE_PATH
	std::ofstream file(OUTPUT_FILE_PATH);
	Json json = JsonObj{{"cppGraph", cppGraph}, {"highlights", globalHiglights}};
	file << json << std::endl;
#endif
	return graphInstruction;
}
