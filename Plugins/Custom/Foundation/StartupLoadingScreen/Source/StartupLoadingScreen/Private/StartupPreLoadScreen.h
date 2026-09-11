// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "PreLoadScreenBase.h"

/**
 * 引擎加载阶段的界面处理对象，负责控件及显示状态的生命周期。
 */
class FStartupPreLoadScreen : public FPreLoadScreenBase
{
public:
	//~IPreLoadScreen 接口
	virtual void Init() override;
	virtual void OnPlay(TWeakPtr<SWindow> TargetWindow) override;
	virtual void OnStop() override;
	virtual void CleanUp() override;
	virtual EPreLoadScreenTypes GetPreLoadScreenType() const override { return EPreLoadScreenTypes::EngineLoadingScreen; }
	virtual TSharedPtr<SWidget> GetWidget() override { return EngineLoadingWidget; }
	//~IPreLoadScreen 接口结束

	// 仅在游戏线程查询，用于模块提前关闭时判断是否需要等待当前界面结束。
	bool IsPlaying() const { return bIsPlaying; }

private:
	TSharedPtr<SWidget> EngineLoadingWidget;
	bool bIsPlaying = false;
};
