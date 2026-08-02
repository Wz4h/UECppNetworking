// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson19/NetLesson19RelevancySpawner.h"

#include "NetLessons/Lesson19/NetLesson19RelevancyCube.h"

ANetLesson19RelevancySpawner::ANetLesson19RelevancySpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	CubeClass = ANetLesson19RelevancyCube::StaticClass();
}

void ANetLesson19RelevancySpawner::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority() || !CubeClass)
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

		ANetLesson19RelevancyCube* Cube = GetWorld()->SpawnActor<ANetLesson19RelevancyCube>(
			CubeClass,
			SpawnLocation,
			GetActorRotation(),
			SpawnParams
		);

		ConfigureCube(Cube);
	}
}

void ANetLesson19RelevancySpawner::ConfigureCube(ANetLesson19RelevancyCube* Cube) const
{
	if (!Cube)
	{
		return;
	}

	Cube->SetNetUpdateFrequency(CubeNetUpdateFrequency);
	Cube->SetMinNetUpdateFrequency(FMath::Min(2.f, CubeNetUpdateFrequency));

	if (RelevancyMode == ENetLesson19RelevancyMode::AlwaysRelevant)
	{
		Cube->bAlwaysRelevant = true;
	}
	else
	{
		Cube->bAlwaysRelevant = false;
		Cube->SetNetCullDistanceSquared(FMath::Square(CullDistance));
	}
}
