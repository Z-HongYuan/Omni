// Copyright © 2026 张鸿源. All Rights Reserved.

#include "StartupPreLoadScreen.h"
#include "Misc/App.h"
#include "StartupPreLoadingScreenWidget.h"

void FStartupPreLoadScreen::Init()
{
	if (!GIsEditor && FApp::CanEverRender())
	{
		EngineLoadingWidget = SNew(SStartupPreLoadingScreenWidget);
	}
}

void FStartupPreLoadScreen::OnPlay(TWeakPtr<SWindow> TargetWindow)
{
	FPreLoadScreenBase::OnPlay(TargetWindow);
	bIsPlaying = true;
}

void FStartupPreLoadScreen::OnStop()
{
	bIsPlaying = false;
	FPreLoadScreenBase::OnStop();
}

void FStartupPreLoadScreen::CleanUp()
{
	// 管理器在停止界面后调用；允许重复清理。
	bIsPlaying = false;
	EngineLoadingWidget.Reset();
	OwningWindow.Reset();
	FPreLoadScreenBase::CleanUp();
}
