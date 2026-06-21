// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson17/NetLesson17GameMode.h"

#include "NetLessons/Lesson17/NetLesson17Character.h"
#include "NetLessons/Lesson17/NetLesson17PlayerController.h"

ANetLesson17GameMode::ANetLesson17GameMode()
{
	DefaultPawnClass = nullptr;
	PlayerControllerClass = ANetLesson17PlayerController::StaticClass();
}

void ANetLesson17GameMode::BeginPlay()
{
	Super::BeginPlay();

	// Lesson17 使用独立 Actor 做演示对象，不依赖 Pawn Possess 流程。
	if (HasAuthority() && GetWorld())
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		GetWorld()->SpawnActor<ANetLesson17Character>(ANetLesson17Character::StaticClass(), FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator, SpawnParameters);
	}
}
