// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "./DataTypes/Singleton.h"

#include "Widgets/Notifications/SNotificationList.h"

enum EANA_ReleaseStep
{
	NONE,
	AUTO_ARRANGE_THANKS,
	AUTO_ARRANGE_ISSUE,
	AUTO_ARRANGE_BETA,
};

class ANA_ReleaseNotifier : public Singleton<ANA_ReleaseNotifier>
{
public:
	void RegisterCheckVersion();
	void CheckVersion();
	void ShowNotification(EANA_ReleaseStep step);

private:
	void ShowAutoArrangeThanksNotification();
	void ShowAutoArrangeIssueNotification();
	void ShowAutoArrangeBetaNotification();
	void ShowNoneNotification();

	TWeakPtr<SNotificationItem> releaseNotifPtr;
	EANA_ReleaseStep completeActionType = EANA_ReleaseStep::NONE;
};
