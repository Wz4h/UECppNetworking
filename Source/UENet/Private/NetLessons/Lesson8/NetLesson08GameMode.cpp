// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson8/NetLesson08GameMode.h"

#include "NetLessons/Lesson8/NetLesson08Character.h"
#include "NetLessons/Lesson8/NetLesson08DoorActor.h"

ANetLesson08GameMode::ANetLesson08GameMode()
{
	DefaultPawnClass = ANetLesson08Character::StaticClass();
}

void ANetLesson08GameMode::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->SpawnActor<ANetLesson08DoorActor>(
		ANetLesson08DoorActor::StaticClass(),
		FVector(350.f, 0.f, 160.f),
		FRotator::ZeroRotator
	);
}
