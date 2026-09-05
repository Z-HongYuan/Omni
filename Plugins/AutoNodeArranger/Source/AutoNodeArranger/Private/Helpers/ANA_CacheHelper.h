// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "./DataTypes/ANA_CacheData.h"
#include "./DataTypes/Singleton.h"

class SGraphPanel;

struct CppGraph;

class ANA_CacheHelper : public Singleton<ANA_CacheHelper>
{
public:
	ANA_ReleaseNotifierData GetReleaseNotifierData();
	void SetReleaseNotifierData(const ANA_ReleaseNotifierData& releaseNotifierData);

	ANA_GraphData GetGraphData(const SGraphPanel* graphPanel);
	void SetGraphData(const SGraphPanel* graphPanel, const ANA_GraphData& graphData);
	void SetGraphData(const FGuid& graphPanelGuid, const ANA_GraphData& graphData);

	void SetDebugCppGraph(const CppGraph& cppGraph);

	const FString& GetDebugFolderPath();

private:
	const FString& GetPluginDir();
	const FString& GetProjectSavedDir();

	const FString& GetReleaseNotifierFilePath();
	const FString& GetGraphDataFolderPath();

	FString GetGraphDataFilePath(const SGraphPanel* graphPanel);
	FString GetGraphDataFilePath(const FGuid& graphPanelGuid);
	const FString& GetDebugCppGraphFilePath();
};
