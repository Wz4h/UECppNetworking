// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson6/NetLesson06GameMode.h"

#include "NetLessons/Lesson6/NetLesson06Character.h"
#include "NetLessons/Lesson6/NetLesson06ScaleActor.h"

ANetLesson06GameMode::ANetLesson06GameMode()
{
	DefaultPawnClass = ANetLesson06Character::StaticClass();
}

void ANetLesson06GameMode::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->SpawnActor<ANetLesson06ScaleActor>(
		ANetLesson06ScaleActor::StaticClass(),
		FVector(300.f, 0.f, 120.f),
		FRotator::ZeroRotator
	);
}
