// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Data/GameUserSession_SearchSessionRequest.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameUserSession_SearchSessionRequest)

void UGameUserSession_SearchSessionRequest::NotifySearchFinished(bool bSucceeded, const FText& ErrorMessage)
{
	OnSearchFinished.Broadcast(bSucceeded, ErrorMessage);
	K2_OnSearchFinished.Broadcast(bSucceeded, ErrorMessage);
}
