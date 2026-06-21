// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson15/NetLesson15GameMode.h"

#include "NetLessons/Lesson15/NetLesson15Character.h"
#include "NetLessons/Lesson15/NetLesson15PlayerState.h"
#include "NetLessons/Lesson15/NetLesson15TeamActor.h"

ANetLesson15GameMode::ANetLesson15GameMode()
{
	DefaultPawnClass = ANetLesson15Character::StaticClass();
	PlayerStateClass = ANetLesson15PlayerState::StaticClass();
}

void ANetLesson15GameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		World->SpawnActor<ANetLesson15TeamActor>(
			ANetLesson15TeamActor::StaticClass(),
			FVector(520.f, 0.f, 80.f),
			FRotator::ZeroRotator
		);
	}
}

void ANetLesson15GameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (ANetLesson15PlayerState* LessonPlayerState = NewPlayer ? NewPlayer->GetPlayerState<ANetLesson15PlayerState>() : nullptr)
	{
		LessonPlayerState->SetTeamId(NextTeamId);
		NextTeamId = NextTeamId == 1 ? 2 : 1;
	}

	if (ANetLesson15Character* LessonCharacter = NewPlayer ? Cast<ANetLesson15Character>(NewPlayer->GetPawn()) : nullptr)
	{
		LessonCharacter->UpdateLessonText();
	}
}
