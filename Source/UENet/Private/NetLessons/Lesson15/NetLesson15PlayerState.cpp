// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson15/NetLesson15PlayerState.h"

#include "Net/UnrealNetwork.h"
#include "NetLessons/Lesson15/NetLesson15Character.h"

void ANetLesson15PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetLesson15PlayerState, TeamId);
}

void ANetLesson15PlayerState::SetTeamId(int32 InTeamId)
{
	TeamId = InTeamId;

	if (ANetLesson15Character* LessonCharacter = GetPawn<ANetLesson15Character>())
	{
		LessonCharacter->UpdateLessonText();
	}
}

int32 ANetLesson15PlayerState::GetTeamId() const
{
	return TeamId;
}

void ANetLesson15PlayerState::OnRep_TeamId()
{
	if (ANetLesson15Character* LessonCharacter = GetPawn<ANetLesson15Character>())
	{
		LessonCharacter->UpdateLessonText();
	}
}
