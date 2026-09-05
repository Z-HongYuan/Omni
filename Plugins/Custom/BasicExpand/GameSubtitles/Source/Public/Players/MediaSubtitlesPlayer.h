// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Tickable.h"

#include "UObject/ObjectPtr.h"
#include "UObject/WeakObjectPtr.h"
#include "MediaSubtitlesPlayer.generated.h"

#define UE_API GAMESUBTITLES_API

class UMediaPlayer;
class UOverlays;
struct FFrame;

/**
 * 用于媒体字幕的游戏专用播放器。它需要与媒体播放器（Media Player）一同存在，
 * 并且其 Play() / Pause() / Stop() 方法要与媒体播放器的
 * 对应方法在同一时间被调用。
 */
UCLASS(MinimalAPI, BlueprintType)
class UMediaSubtitlesPlayer
	: public UObject
	  , public FTickableGameObject
{
	GENERATED_UCLASS_BODY()

public:
	/** 此播放器要使用的字幕。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Subtitles Source")
	TObjectPtr<UOverlays> SourceSubtitles;

public:
	UE_API virtual void BeginDestroy() override;

	/** 开始播放当前设置的字幕。 */
	UFUNCTION(BlueprintCallable, Category="Game Subtitles|Subtitles Player")
	UE_API void Play();

	/** 停止字幕播放器。 */
	UFUNCTION(BlueprintCallable, Category="Game Subtitles|Subtitles Player")
	UE_API void Stop();

	/** 使用新的字幕集设置数据源。 */
	UFUNCTION(BlueprintCallable, Category="Game Subtitles|Subtitles Player")
	UE_API void SetSubtitles(UOverlays* Subtitles);

	/** 将字幕播放绑定到媒体播放器的 Tick 上。 */
	UFUNCTION(BlueprintCallable, Category="Game Subtitles|Subtitles Player")
	UE_API void BindToMediaPlayer(UMediaPlayer* InMediaPlayer);

public:
	//~ FTickableGameObject 接口
	UE_API virtual void Tick(float DeltaSeconds) override;
	virtual ETickableTickType GetTickableTickType() const override { return (HasAnyFlags(RF_ClassDefaultObject) ? ETickableTickType::Never : ETickableTickType::Always); }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UMediaSubtitlesPlayer, STATGROUP_Tickables); }

private:
	/** 对媒体播放器的引用 */
	TWeakObjectPtr<class UMediaPlayer> MediaPlayer;

	/** 字幕当前是否正在显示 */
	bool bEnabled;
};

#undef UE_API
