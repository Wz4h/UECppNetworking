// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson20/NetLesson20DormancyDoorSpawner.h"

#include "NetLessons/Lesson20/NetLesson20DormancyDoor.h"

ANetLesson20DormancyDoorSpawner::ANetLesson20DormancyDoorSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	DoorClass = ANetLesson20DormancyDoor::StaticClass();
}

void ANetLesson20DormancyDoorSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority() || !DoorClass)
	{
		return;
	}

	const int32 GridWidth = FMath::CeilToInt(FMath::Sqrt(static_cast<float>(SpawnCount)));
	const FVector Origin = GetActorLocation();

	for (int32 Index = 0; Index < SpawnCount; ++Index)
	{
		const int32 Row = Index / GridWidth;
		const int32 Column = Index % GridWidth;
		const FVector SpawnLocation = Origin + FVector(Column * GridSpacing, Row * GridSpacing, 0.f);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (ANetLesson20DormancyDoor* Door = GetWorld()->SpawnActor<ANetLesson20DormancyDoor>(DoorClass, SpawnLocation, GetActorRotation(), SpawnParams))
		{
			Door->SetUseDormancy(bUseDormancy);
		}
	}
}
