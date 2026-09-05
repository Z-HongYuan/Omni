// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

using ANA_TickHandler = TFunction<void(const float DeltaTime)>;
using ANA_KeyHandler = TFunction<void(const FKeyEvent& InKeyEvent)>;
using ANA_EventID = int64;

template <typename THandler> class ANA_EventDispatcher
{
public:
	void Add(ANA_EventID id, const THandler& handler) { toAddHandlerList.Add(TPair<ANA_EventID, THandler>(id, handler)); }

	void Remove(ANA_EventID idToRemove) { toRemoveIdList.Add(idToRemove); }

	template <typename... Args> void Execute(Args&&... args)
	{
		for (auto& toRemoveId : toRemoveIdList) handlerMap.Remove(toRemoveId);
		toRemoveIdList.Empty();
		for (auto& toAddHandler : toAddHandlerList) handlerMap.Add(toAddHandler);
		toAddHandlerList.Empty();
		for (auto& [_, handler] : handlerMap) handler(args...);
	}

private:
	TMap<ANA_EventID, THandler> handlerMap;

	TArray<TPair<ANA_EventID, THandler>> toAddHandlerList;
	TArray<ANA_EventID> toRemoveIdList;
};
