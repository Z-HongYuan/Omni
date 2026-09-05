// Copyright 2024 bstt, Inc. All Rights Reserved.

#include "ANA_CommandHelper.h"

#include "../../Config/ANA_EditorConfig.h"
#include "../../CppGraphArranger/CppGraphArranger.h"
#include "../../CppGraphArranger/NodeSelector.h"
#include "../../Debug/Log.h"
#include "../ANA_CacheHelper.h"
#include "../Arranger/ANA_ConfigExporter.h"
#include "../Arranger/ANA_GraphAnimator.h"
#include "../Arranger/ANA_GraphExporter.h"
#include "../Arranger/ANA_InstructionsExecutor.h"
#include "./ANA_GraphHelper.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#include "../../CppGraphArranger/main.h"
#include "../../OldCppGraphArranger/ArrangerMain.h"

void ANA_CommandHelper::ArrangeNodesStraight()
{
	// bAutoArrange, bCompact, bCenterize
	ArrangeNodes(false, false);
}

void ANA_CommandHelper::ArrangeNodesCenter()
{
	// bAutoArrange, bCompact, bCenterize
	ArrangeNodes(false, true);
}

void ANA_CommandHelper::ArrangeNodesCompact()
{
	// bAutoArrange, bCompact, bCenterize
	ArrangeNodes(true, false); // last bCenterize will be used
}

void ANA_CommandHelper::SelectConnectedGraph()
{
	INIT_GRAPH_PANEL();
	auto cppConfig = ANA_ConfigExporter::ExportConfig(graphConfig, graphType, false, false);
	ANA_GraphExport graphExport(graphPanel);
	ANA_GraphExporter::ExportGraph(cppConfig, graphExport, false);
	auto& cppGraph = graphExport.cppGraph;
	NodeSelector nodeSelector(cppGraph);
	auto editorConfig = UANA_EditorConfig::Get();
	SelectInstruction instruction = nodeSelector.SelectConnected(editorConfig->bProgressiveSelection);
	ANA_InstructionsExecutor::ExecuteSelectInstruction(graphExport, instruction);
}

void ANA_CommandHelper::AddCustomConfig()
{
	TSharedPtr<SGraphPanel> shrGraphPanel = ANA_GraphHelper::GetCurrentGraphPanel();
	if (!shrGraphPanel.IsValid())
	{
		FNotificationInfo notifWarning(NSLOCTEXT("AutoNodeArranger", "AddCustomConfigCommand", ""));
		FString message = FString("No graph panel detected");
		notifWarning.Text = FText::FromString(message);
		notifWarning.ExpireDuration = 3.0f;
		notifWarning.Image = FCoreStyle::Get().GetBrush(TEXT("MessageLog.Warning"));
		FSlateNotificationManager::Get().AddNotification(notifWarning);
		UE_LOG(LogAutoNodeArranger, Warning, TEXT("%s"), *message);
		return;
	}
	auto graphClassName = shrGraphPanel->GetGraphObj()->GetClass()->GetFName().ToString();
	if (UANA_EditorConfig::ContainsGraphConfig(graphClassName))
	{
		FNotificationInfo notifInfo(NSLOCTEXT("AutoNodeArranger", "AddCustomConfigCommand", ""));
		FString message = graphClassName + FString(" has already a custom config");
		notifInfo.Text = FText::FromString(message);
		notifInfo.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(notifInfo);
		UE_LOG(LogAutoNodeArranger, Warning, TEXT("%s"), *message);
		return;
	}
	UANA_EditorConfig::AddGraphConfig(graphClassName);
	FNotificationInfo notifInfo(NSLOCTEXT("AutoNodeArranger", "AddCustomConfigCommand", ""));
	FString message = graphClassName + FString(" added");
	notifInfo.Text = FText::FromString(message);
	notifInfo.ExpireDuration = 3.0f;
	FSlateNotificationManager::Get().AddNotification(notifInfo);
	UE_LOG(LogAutoNodeArranger, Warning, TEXT("%s"), *message);
}

void ANA_CommandHelper::ToggleUseArrangement()
{
	UANA_EditorConfig::Get()->bUseArrangement = !UANA_EditorConfig::Get()->bUseArrangement;
	FNotificationInfo notifInfo(NSLOCTEXT("AutoNodeArranger", "AddCustomConfigCommand", ""));
	FString message = FString("Arrangement ") + (UANA_EditorConfig::Get()->bUseArrangement ? " enabled" : "disabled");
	notifInfo.Text = FText::FromString(message);
	notifInfo.ExpireDuration = 3.0f;
	FSlateNotificationManager::Get().AddNotification(notifInfo);
	UE_LOG(LogAutoNodeArranger, Warning, TEXT("%s"), *message);
}

void ANA_CommandHelper::ArrangeNodes(bool bCompact, bool bCenterize)
{
	INIT_GRAPH_PANEL();
	if (isForbiddenGraph)
	{
		FNotificationInfo notifWarning(NSLOCTEXT("AutoNodeArranger", "ArrangeCommand", ""));
		FString message = FString("Arrange command disabled for this graph type");
		notifWarning.ExpireDuration = 3.0f;
		notifWarning.Text = FText::FromString(message);
		notifWarning.Image = FCoreStyle::Get().GetBrush(TEXT("MessageLog.Warning"));
		FSlateNotificationManager::Get().AddNotification(notifWarning);
		UE_LOG(LogAutoNodeArranger, Warning, TEXT("%s"), *message);
		return;
	}

	// initialized with parameters value
	static bool bLastCompact = bCompact;
	static bool bLastCenterize = bCenterize;

	if (bCompact)
		bCenterize = bLastCenterize && (graphType != EANA_GraphType::Blueprint); // no compact centerize for blueprintGraph

	bLastCompact = bCompact;
	bLastCenterize = bCenterize;

	auto cppConfig = ANA_ConfigExporter::ExportConfig(graphConfig, graphType, bCompact, bCenterize);
	ANA_GraphExport graphExport(graphPanel);
	if (!ANA_GraphExporter::ExportGraph(cppConfig, graphExport, true))
		return; // ANA_CommandHelper::ArrangeNodes will be called by ANA_NodeRegister::RegisterGraph
	auto& cppGraph = graphExport.cppGraph;

	try
	{
		// bUseArrangement considered within oldMainFn/mainFn
		GraphInstruction graphInstruction
			= !UANA_EditorConfig::Get()->bUseBeta || bCenterize || (graphType != EANA_GraphType::Blueprint) ? oldMainFn(cppGraph)
																											: mainFn(cppGraph);

		if (cppConfig.bUseArrangement) ANA_GraphAnimator::AnimateGraphInstruction(graphExport, graphInstruction);
		else
		{
			static TSharedPtr<SNotificationItem> notifInfoPtr;
			if (notifInfoPtr)
			{
				notifInfoPtr->SetFadeOutDuration(0.f);
				notifInfoPtr->Fadeout();
				notifInfoPtr = nullptr;
			}
			FNotificationInfo notifInfo(NSLOCTEXT("AutoNodeArranger", "AddCustomConfigCommand", ""));
			FString message = FString("Arragement still disabled\n(ctrl + shift + space to put it back)");
			notifInfo.Text = FText::FromString(message);
			notifInfo.FadeOutDuration = 0.2f;
			notifInfo.bFireAndForget = false;

			notifInfo.Hyperlink = FSimpleDelegate::CreateLambda(
				[cppGraph]() // make a copy
				{
					const FString DocsURL
						= TEXT("https://github.com/bstt/AutoNodeArranger/issues/"
							   "new?assignees=&labels=bug&projects=&template=bug_report.yaml&title=%5BBug%5D%3A+");
					FPlatformProcess::LaunchURL(*DocsURL, nullptr, nullptr);
					ANA_CacheHelper::Get().SetDebugCppGraph(cppGraph);
					FPlatformProcess::ExploreFolder(*ANA_CacheHelper::Get().GetDebugFolderPath());
				});
			notifInfo.HyperlinkText = FText::FromString("Raise issue...");

			notifInfo.ButtonDetails.Add(FNotificationButtonInfo(FText::FromString("Close"),
				FText::FromString("Close"),
				FSimpleDelegate::CreateLambda(
					[]()
					{
						if (!notifInfoPtr) return;
						notifInfoPtr->SetFadeOutDuration(0.f);
						notifInfoPtr->Fadeout();
						notifInfoPtr = nullptr;
					}),
				SNotificationItem::ECompletionState::CS_None));

			notifInfoPtr = FSlateNotificationManager::Get().AddNotification(notifInfo);
			UE_LOG(LogAutoNodeArranger, Warning, TEXT("%s"), *message);
		}
	}
	catch (const std::runtime_error& err)
	{
		FNotificationInfo notifWarning(NSLOCTEXT("AutoNodeArranger", "ArrangeCommand", ""));
		FString message = FString("An error occured while arrangement: ") + err.what();
		notifWarning.ExpireDuration = 3.0f;
		notifWarning.Text = FText::FromString(message);
		notifWarning.Image = FCoreStyle::Get().GetBrush(TEXT("MessageLog.Warning"));
		FSlateNotificationManager::Get().AddNotification(notifWarning);
		UE_LOG(LogAutoNodeArranger, Warning, TEXT("%s"), *message);
	}
	catch (...)
	{
		FNotificationInfo notifWarning(NSLOCTEXT("AutoNodeArranger", "ArrangeCommand", ""));
		FString message = FString("An unknown error occured while arrangement");
		notifWarning.ExpireDuration = 3.0f;
		notifWarning.Text = FText::FromString(message);
		notifWarning.Image = FCoreStyle::Get().GetBrush(TEXT("MessageLog.Warning"));
		FSlateNotificationManager::Get().AddNotification(notifWarning);
		UE_LOG(LogAutoNodeArranger, Warning, TEXT("%s"), *message);
	}
}
