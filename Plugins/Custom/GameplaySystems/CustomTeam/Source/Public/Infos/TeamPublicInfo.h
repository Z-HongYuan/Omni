// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "TeamInfoBase.h"
#include "TeamPublicInfo.generated.h"

#define UE_API CUSTOMTEAM_API

class UTeamDisplayAssetBase;

/** 队伍公开信息，保存仅初始化时复制的展示资产。 */
UCLASS(MinimalAPI)
class ATeamPublicInfo : public ATeamInfoBase
{
	GENERATED_BODY()

public:
	ATeamPublicInfo(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UTeamDisplayAssetBase* GetTeamDisplayAsset() const { return TeamDisplayAsset; }

protected:
	UFUNCTION()
	void OnRep_TeamDisplayAsset();

	void SetTeamDisplayAsset(TObjectPtr<UTeamDisplayAssetBase> NewDisplayAsset);

	UPROPERTY(ReplicatedUsing=OnRep_TeamDisplayAsset)
	TObjectPtr<UTeamDisplayAssetBase> TeamDisplayAsset;
};

#undef UE_API
