// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson11/NetLesson11GameMode.h"

#include "NetLessons/Lesson11/NetLesson11Character.h"
#include "NetLessons/Lesson11/NetLesson11FrameworkActor.h"

ANetLesson11GameMode::ANetLesson11GameMode()
{
	DefaultPawnClass = ANetLesson11Character::StaticClass();
}

void ANetLesson11GameMode::BeginPlay()
{
	Super::BeginPlay();

	SpawnFrameworkActors();
}

void ANetLesson11GameMode::SpawnFrameworkActors()
{
	UWorld* World = GetWorld();
	if (!World || !HasAuthority())
	{
		return;
	}

	const FActorSpawnParameters SpawnParams;

	ANetLesson11FrameworkActor* Door = World->SpawnActor<ANetLesson11FrameworkActor>(
		ANetLesson11FrameworkActor::StaticClass(),
		FVector(350.f, -260.f, 80.f),
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (Door)
	{
		Door->SetActorLabel(TEXT("Lesson11_Door"));
		Door->Tags.Add(TEXT("Lesson11_Door"));
		Door->ConfigureDemoActor(TEXT("Door Actor"), 1, TEXT("Spawn/Destroy use ActorChannel"));
	}

	ANetLesson11FrameworkActor* Box = World->SpawnActor<ANetLesson11FrameworkActor>(
		ANetLesson11FrameworkActor::StaticClass(),
		FVector(350.f, 0.f, 80.f),
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (Box)
	{
		Box->SetActorLabel(TEXT("Lesson11_Box"));
		Box->Tags.Add(TEXT("Lesson11_Box"));
		Box->ConfigureDemoActor(TEXT("Box Actor"), 2, TEXT("Replication sends Value"));
	}

	ANetLesson11FrameworkActor* Coin = World->SpawnActor<ANetLesson11FrameworkActor>(
		ANetLesson11FrameworkActor::StaticClass(),
		FVector(350.f, 260.f, 80.f),
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (Coin)
	{
		Coin->SetActorLabel(TEXT("Lesson11_Coin"));
		Coin->Tags.Add(TEXT("Lesson11_Coin"));
		Coin->SetNetCullDistanceSquared(FMath::Square(900.f));
		Coin->ConfigureDemoActor(TEXT("Coin Actor"), 3, TEXT("Relevancy keeps/closes Channel"));
	}
}
