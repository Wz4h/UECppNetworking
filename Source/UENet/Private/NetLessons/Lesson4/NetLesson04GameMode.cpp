// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson4/NetLesson04GameMode.h"

#include "NetLessons/Lesson4/NetLesson04Character.h"
#include "NetLessons/Lesson4/NetLesson04ScaleActor.h"

ANetLesson04GameMode::ANetLesson04GameMode()
{
	DefaultPawnClass = ANetLesson04Character::StaticClass();
}

void ANetLesson04GameMode::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->SpawnActor<ANetLesson04ScaleActor>(
		ANetLesson04ScaleActor::StaticClass(),
		FVector(300.f, 0.f, 120.f),
		FRotator::ZeroRotator
	);
}
