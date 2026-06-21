// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson12/NetLesson12GameMode.h"

#include "NetLessons/Lesson12/NetLesson12Character.h"
#include "NetLessons/Lesson12/NetLesson12RelevancyActor.h"

ANetLesson12GameMode::ANetLesson12GameMode()
{
	DefaultPawnClass = ANetLesson12Character::StaticClass();
}

void ANetLesson12GameMode::BeginPlay()
{
	Super::BeginPlay();

	SpawnDistanceActor();
}

void ANetLesson12GameMode::SpawnDistanceActor()
{
	UWorld* World = GetWorld();
	if (!World || !HasAuthority())
	{
		return;
	}

	const FActorSpawnParameters SpawnParams;

	ANetLesson12RelevancyActor* DistanceActor = World->SpawnActor<ANetLesson12RelevancyActor>(
		ANetLesson12RelevancyActor::StaticClass(),
		FVector(520.f, 0.f, 80.f),
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (DistanceActor)
	{
		DistanceActor->Tags.Add(TEXT("Lesson12_Distance"));
		DistanceActor->SetNetCullDistanceSquared(FMath::Square(900.f));
		DistanceActor->ConfigureDemoActor(TEXT("Distance Actor"), TEXT("Near / Far Cull"), 1);
	}
}
