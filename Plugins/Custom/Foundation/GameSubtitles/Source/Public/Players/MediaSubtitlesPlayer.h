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
 * 根据绑定的媒体播放器时间读取字幕。调用方在媒体开始和停止时同步调用 Play() / Stop()。
 * 暂停和跳转随媒体时间同步，无需独立的 Pause() 接口；字幕播放器自身需由调用方持有。
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

	/** 指定字幕读取时间所依据的媒体播放器；该调用不自动开始播放字幕。 */
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
