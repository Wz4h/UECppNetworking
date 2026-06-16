// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson12/NetLesson12PlayerState.h"

#include "Net/UnrealNetwork.h"
#include "NetLessons/Lesson12/NetLesson12Character.h"

void ANetLesson12PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetLesson12PlayerState, TeamId);
}

void ANetLesson12PlayerState::SetTeamId(int32 InTeamId)
{
	TeamId = InTeamId;

	if (ANetLesson12Character* LessonCharacter = GetPawn<ANetLesson12Character>())
	{
		LessonCharacter->UpdateTeamText();
	}
}

int32 ANetLesson12PlayerState::GetTeamId() const
{
	return TeamId;
}

void ANetLesson12PlayerState::OnRep_TeamId()
{
	if (ANetLesson12Character* LessonCharacter = GetPawn<ANetLesson12Character>())
	{
		LessonCharacter->UpdateTeamText();
	}
}
