// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson10/NetLesson10GameMode.h"

#include "NetLessons/Lesson10/NetLesson10Character.h"
#include "NetLessons/Lesson10/NetLesson10DoorActor.h"

ANetLesson10GameMode::ANetLesson10GameMode()
{
	DefaultPawnClass = ANetLesson10Character::StaticClass();
}

void ANetLesson10GameMode::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->SpawnActor<ANetLesson10DoorActor>(
		ANetLesson10DoorActor::StaticClass(),
		FVector(350.f, -80.f, 160.f),
		FRotator::ZeroRotator
	);
}
