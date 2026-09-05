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
