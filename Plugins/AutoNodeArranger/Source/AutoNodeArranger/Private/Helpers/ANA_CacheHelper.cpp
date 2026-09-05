// Copyright 2024 bstt, Inc. All Rights Reserved.

#include "ANA_CacheHelper.h"
#include "../CppGraphArranger/Helpers/AllJsonConverters.h"
#include "../Debug/Log.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "SGraphPanel.h"

const FString& ANA_CacheHelper::GetPluginDir()
{
	static FString pluginDir
		= FPaths::ConvertRelativePathToFull(IPluginManager::Get().FindPlugin("AutoNodeArranger")->GetBaseDir() + "/Saved");
	return pluginDir;
}

const FString& ANA_CacheHelper::GetProjectSavedDir()
{
	static FString projectSavedDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() + "/AutoNodeArranger");
	return projectSavedDir;
}

const FString& ANA_CacheHelper::GetReleaseNotifierFilePath()
{
	static FString releaseNotifierPath = GetPluginDir() + "/releaseNotifier.json";
	return releaseNotifierPath;
}

const FString& ANA_CacheHelper::GetGraphDataFolderPath()
{
	static FString graphDataFolderPath = GetProjectSavedDir() + "/GraphData";
	return graphDataFolderPath;
}

const FString& ANA_CacheHelper::GetDebugFolderPath()
{
	static FString debugFolderPath = GetProjectSavedDir() + "/Debug";
	return debugFolderPath;
}

FString ANA_CacheHelper::GetGraphDataFilePath(const SGraphPanel* graphPanel)
{
	return GetGraphDataFilePath(graphPanel->GetGraphObj()->GraphGuid);
}

FString ANA_CacheHelper::GetGraphDataFilePath(const FGuid& graphPanelGuid)
{
	return GetGraphDataFolderPath() + "/" + graphPanelGuid.ToString() + ".json";
}

const FString& ANA_CacheHelper::GetDebugCppGraphFilePath()
{
	static FString debugCppGraphFilePath = GetDebugFolderPath() + "/cppGraph.json";
	return debugCppGraphFilePath;
}

ANA_ReleaseNotifierData ANA_CacheHelper::GetReleaseNotifierData()
{
	const FString& path = GetReleaseNotifierFilePath();
	ANA_ReleaseNotifierData data;

	if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*path))
	{
		std::string sPath = std::string(TCHAR_TO_UTF8(*path));
		Json json;

		if (Json::tryParseFile(sPath, json)) return json; // parse succeed
		else
			UE_LOG(LogAutoNodeArranger, Warning, TEXT("Cannot load release notifier data"));
	}
	else
		UE_LOG(LogAutoNodeArranger, Log, TEXT("Release notifier data not found here: %s"), *path);

	SetReleaseNotifierData(data);

	return data;
}

void ANA_CacheHelper::SetReleaseNotifierData(const ANA_ReleaseNotifierData& data)
{
	const FString& path = GetReleaseNotifierFilePath();
	std::string sPath = std::string(TCHAR_TO_UTF8(*path));
	std::string dataStr = Json(data).toString();
	FString str = FString(dataStr.data());

	if (!FFileHelper::SaveStringToFile(str, *path))
		UE_LOG(LogAutoNodeArranger, Warning, TEXT("Cannot write release notifier data here: %s"), *path);
}

ANA_GraphData ANA_CacheHelper::GetGraphData(const SGraphPanel* graphPanel)
{
	FString path = GetGraphDataFilePath(graphPanel);
	ANA_GraphData data;

	if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*path))
	{
		std::string sPath = std::string(TCHAR_TO_UTF8(*path));
		Json json;

		if (Json::tryParseFile(sPath, json)) return json; // parse succeed
		else
			UE_LOG(LogAutoNodeArranger, Warning, TEXT("Cannot load graph data"));
	}
	else
		UE_LOG(LogAutoNodeArranger, Log, TEXT("Graph data not found here: %s"), *path);

	SetGraphData(graphPanel, data);

	return data;
}

void ANA_CacheHelper::SetGraphData(const SGraphPanel* graphPanel, const ANA_GraphData& graphData)
{
	SetGraphData(graphPanel->GetGraphObj()->GraphGuid, graphData);
}

void ANA_CacheHelper::SetGraphData(const FGuid& graphPanelGuid, const ANA_GraphData& graphData)
{
	FString path = GetGraphDataFilePath(graphPanelGuid);
	std::string sPath = std::string(TCHAR_TO_UTF8(*path));
	std::string dataStr = Json(graphData).toString();
	FString str = FString(dataStr.data());

	if (!FFileHelper::SaveStringToFile(str, *path))
		UE_LOG(LogAutoNodeArranger, Warning, TEXT("Cannot write graph data here: %s"), *path);
}

void ANA_CacheHelper::SetDebugCppGraph(const CppGraph& cppGraph)
{
	Json json = JsonObj{{"cppGraph", cppGraph}};

	const FString& path = GetDebugCppGraphFilePath();
	std::string s = json.toString();
	FString str = FString(s.c_str());

	if (!FFileHelper::SaveStringToFile(str, *path))
		UE_LOG(LogAutoNodeArranger, Warning, TEXT("Cannot write cpp graph data here: %s"), *path);
}
