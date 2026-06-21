// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson16/NetLesson16GameMode.h"

#include "NetLessons/Lesson16/NetLesson16Character.h"
#include "NetLessons/Lesson16/NetLesson16PlayerController.h"

ANetLesson16GameMode::ANetLesson16GameMode()
{
	DefaultPawnClass = nullptr;
	PlayerControllerClass = ANetLesson16PlayerController::StaticClass();
}

void ANetLesson16GameMode::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World || !HasAuthority())
	{
		return;
	}

	World->SpawnActor<ANetLesson16Character>(
		ANetLesson16Character::StaticClass(),
		FVector(0.f, 0.f, 100.f),
		FRotator::ZeroRotator
	);
}
