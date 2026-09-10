// Copyright Epic Games, Inc. All Rights Reserved.

#include "Players/MediaSubtitlesPlayer.h"

#include "MediaPlayer.h"
#include "Overlays.h"
#include "Stats/Stats.h"
#include "SubtitleManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MediaSubtitlesPlayer)

UMediaSubtitlesPlayer::UMediaSubtitlesPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	  , MediaPlayer(nullptr)
	  , bEnabled(false)
{
}

void UMediaSubtitlesPlayer::BeginDestroy()
{
	Stop();

	Super::BeginDestroy();
}

void UMediaSubtitlesPlayer::Play()
{
	bEnabled = true;
}

void UMediaSubtitlesPlayer::Stop()
{
	bEnabled = false;

	// 清除此对象的影片字幕
	FSubtitleManager::GetSubtitleManager()->SetMovieSubtitle(this, TArray<FString>());
}

void UMediaSubtitlesPlayer::SetSubtitles(UOverlays* Subtitles)
{
	SourceSubtitles = Subtitles;
	if (!SourceSubtitles)
	{
		// 切换为空字幕源时立即清理，避免保留上一次提交的字幕。
		FSubtitleManager::GetSubtitleManager()->SetMovieSubtitle(this, TArray<FString>());
	}
}

void UMediaSubtitlesPlayer::BindToMediaPlayer(UMediaPlayer* InMediaPlayer)
{
	MediaPlayer = InMediaPlayer;
}

void UMediaSubtitlesPlayer::Tick(float DeltaSeconds)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_UMediaSubtitlesPlayer_Tick);

	if (bEnabled)
	{
		if (!SourceSubtitles)
		{
			// 蓝图可以直接修改公开属性，因此 Tick 也需要处理字幕源被清空的情况。
			// 保留播放状态，后续设置新的字幕源时可以继续跟随媒体时间。
			FSubtitleManager::GetSubtitleManager()->SetMovieSubtitle(this, TArray<FString>());
			return;
		}

		UMediaPlayer* MediaPlayerPtr = MediaPlayer.Get();
		if (MediaPlayerPtr)
		{
			FTimespan CurrentTime = MediaPlayerPtr->GetTime();
			TArray<FOverlayItem> CurrentSubtitles;
			SourceSubtitles->GetOverlaysForTime(CurrentTime, CurrentSubtitles);

			TArray<FString> SubtitlesText;
			for (const FOverlayItem& Subtitle : CurrentSubtitles)
			{
				SubtitlesText.Add(Subtitle.Text);
			}

			FSubtitleManager::GetSubtitleManager()->SetMovieSubtitle(this, SubtitlesText);
		}
		else
		{
			Stop();
		}
	}
}
