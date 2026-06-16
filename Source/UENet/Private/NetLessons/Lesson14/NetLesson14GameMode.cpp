// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson14/NetLesson14GameMode.h"

#include "NetLessons/Lesson14/NetLesson14Character.h"
#include "NetLessons/Lesson14/NetLesson14OwnerActor.h"

ANetLesson14GameMode::ANetLesson14GameMode()
{
	DefaultPawnClass = ANetLesson14Character::StaticClass();
}

void ANetLesson14GameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!NewPlayer)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = NewPlayer;

	if (UWorld* World = GetWorld())
	{
		World->SpawnActor<ANetLesson14OwnerActor>(
			ANetLesson14OwnerActor::StaticClass(),
			FVector(520.f, PrivateActorIndex * 320.f, 80.f),
			FRotator::ZeroRotator,
			SpawnParams
		);
		PrivateActorIndex++;
	}
}
