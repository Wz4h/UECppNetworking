// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson13/NetLesson13GameMode.h"

#include "NetLessons/Lesson13/NetLesson13AlwaysActor.h"
#include "NetLessons/Lesson13/NetLesson13Character.h"

ANetLesson13GameMode::ANetLesson13GameMode()
{
	DefaultPawnClass = ANetLesson13Character::StaticClass();
}

void ANetLesson13GameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		World->SpawnActor<ANetLesson13AlwaysActor>(
			ANetLesson13AlwaysActor::StaticClass(),
			FVector(520.f, 0.f, 80.f),
			FRotator::ZeroRotator
		);
	}
}
