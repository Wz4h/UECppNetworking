// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson9/NetLesson09GameMode.h"

#include "NetLessons/Lesson9/NetLesson09Character.h"
#include "NetLessons/Lesson9/NetLesson09DoorActor.h"

ANetLesson09GameMode::ANetLesson09GameMode()
{
	DefaultPawnClass = ANetLesson09Character::StaticClass();
}

void ANetLesson09GameMode::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->SpawnActor<ANetLesson09DoorActor>(
		ANetLesson09DoorActor::StaticClass(),
		FVector(350.f, 0.f, 160.f),
		FRotator::ZeroRotator
	);
}
