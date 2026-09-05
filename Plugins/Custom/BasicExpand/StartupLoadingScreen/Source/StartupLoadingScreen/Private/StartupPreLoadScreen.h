// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "PreLoadScreenBase.h"

/**
 * 加载界面的处理对象
 */
class FStartupPreLoadScreen : public FPreLoadScreenBase
{
public:
	/** IPreLoadScreen 接口实现 ***/
	virtual void Init() override;
	virtual EPreLoadScreenTypes GetPreLoadScreenType() const override { return EPreLoadScreenTypes::EngineLoadingScreen; }
	virtual TSharedPtr<SWidget> GetWidget() override { return EngineLoadingWidget; }

private:
	TSharedPtr<SWidget> EngineLoadingWidget;
};
