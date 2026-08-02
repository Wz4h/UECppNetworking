// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson21/NetLesson21GameMode.h"

#include "NetLessons/Lesson18/NetLesson18Character.h"

ANetLesson21GameMode::ANetLesson21GameMode()
{
	DefaultPawnClass = ANetLesson18Character::StaticClass();
	AutoSpawnMode = ENetLesson21SpawnMode::Both;
}

void ANetLesson21GameMode::BeginPlay()
{
	Super::BeginPlay();

	SpawnFrequencyDemo();
}

void ANetLesson21GameMode::SpawnFrequencyDemo()
{
	if (!HasAuthority() || !GetWorld())
	{
		return;
	}

	const FTransform SpawnTransform(FRotator::ZeroRotator, FVector::ZeroVector);
	ANetLesson21FrequencyMoverSpawner* Spawner = GetWorld()->SpawnActorDeferred<ANetLesson21FrequencyMoverSpawner>(
		ANetLesson21FrequencyMoverSpawner::StaticClass(),
		SpawnTransform
	);

	if (!Spawner)
	{
		return;
	}

	Spawner->SetSpawnMode(AutoSpawnMode);
	Spawner->FinishSpawning(SpawnTransform);
}

ANetLesson21HighOnlyGameMode::ANetLesson21HighOnlyGameMode()
{
	AutoSpawnMode = ENetLesson21SpawnMode::HighOnly;
}

ANetLesson21LowOnlyGameMode::ANetLesson21LowOnlyGameMode()
{
	AutoSpawnMode = ENetLesson21SpawnMode::LowOnly;
}
