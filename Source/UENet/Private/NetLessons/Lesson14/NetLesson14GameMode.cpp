// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson14/NetLesson14GameMode.h"

#include "GameFramework/PlayerController.h"
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
		const int32 SideIndex = (PrivateActorIndex + 1) / 2;
		const float SideSign = (PrivateActorIndex % 2 == 0) ? 1.f : -1.f;
		const float SpawnY = PrivateActorIndex == 0 ? 0.f : SideSign * SideIndex * 420.f;
		const FVector SpawnLocation(520.f, SpawnY, 80.f);

		World->SpawnActor<ANetLesson14OwnerActor>(
			ANetLesson14OwnerActor::StaticClass(),
			SpawnLocation,
			FRotator::ZeroRotator,
			SpawnParams
		);
		PrivateActorIndex++;
	}
}
