// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson21/NetLesson21FrequencyMoverSpawner.h"

#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/TextRenderActor.h"
#include "NetLessons/Lesson21/NetLesson21FrequencyMover.h"

ANetLesson21FrequencyMoverSpawner::ANetLesson21FrequencyMoverSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	MoverClass = ANetLesson21FrequencyMover::StaticClass();
}

void ANetLesson21FrequencyMoverSpawner::SetSpawnMode(ENetLesson21SpawnMode NewSpawnMode)
{
	SpawnMode = NewSpawnMode;
}

void ANetLesson21FrequencyMoverSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	const FVector Origin = GetActorLocation();

	if (ShouldSpawnHighFrequencyGroup())
	{
		const FVector LabelLocation = Origin + FVector(GetLaneX(true), -600.f, 180.f);
		SpawnLaneLabel(TEXT("High Frequency\n100 Hz"), LabelLocation, FColor::Green);
		SpawnLane(true);
	}

	if (ShouldSpawnLowFrequencyGroup())
	{
		const FVector LabelLocation = Origin + FVector(GetLaneX(false), -600.f, 180.f);
		SpawnLaneLabel(TEXT("Low Frequency\n0.5 Hz"), LabelLocation, FColor::Red);
		SpawnLane(false);
	}
}

bool ANetLesson21FrequencyMoverSpawner::ShouldSpawnHighFrequencyGroup() const
{
	return SpawnMode == ENetLesson21SpawnMode::Both || SpawnMode == ENetLesson21SpawnMode::HighOnly;
}

bool ANetLesson21FrequencyMoverSpawner::ShouldSpawnLowFrequencyGroup() const
{
	return SpawnMode == ENetLesson21SpawnMode::Both || SpawnMode == ENetLesson21SpawnMode::LowOnly;
}

bool ANetLesson21FrequencyMoverSpawner::IsSingleLaneMode() const
{
	return SpawnMode == ENetLesson21SpawnMode::HighOnly || SpawnMode == ENetLesson21SpawnMode::LowOnly;
}

float ANetLesson21FrequencyMoverSpawner::GetLaneX(bool bHighFrequencyGroup) const
{
	if (IsSingleLaneMode())
	{
		return 0.f;
	}

	return bHighFrequencyGroup ? -LaneSpacing * 0.5f : LaneSpacing * 0.5f;
}

void ANetLesson21FrequencyMoverSpawner::SpawnLane(bool bHighFrequencyGroup)
{
	if (!MoverClass)
	{
		return;
	}

	const FVector Origin = GetActorLocation();
	const float LaneX = GetLaneX(bHighFrequencyGroup);
	const float TotalLength = (SpawnCountPerGroup - 1) * ActorSpacing;
	const ENetLesson21FrequencyMode Mode = bHighFrequencyGroup
		? ENetLesson21FrequencyMode::HighFrequency
		: ENetLesson21FrequencyMode::LowFrequency;

	for (int32 Index = 0; Index < SpawnCountPerGroup; ++Index)
	{
		const float LaneY = Index * ActorSpacing - TotalLength * 0.5f;
		const FVector SpawnLocation = Origin + FVector(LaneX, LaneY, 90.f);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (ANetLesson21FrequencyMover* Mover = GetWorld()->SpawnActor<ANetLesson21FrequencyMover>(MoverClass, SpawnLocation, GetActorRotation(), SpawnParams))
		{
			const float PhaseOffset = static_cast<float>(Index) * 0.04f;
			Mover->ConfigureDemo(Mode, MoveDistance, MoveSpeed, PhaseOffset);
		}
	}
}

void ANetLesson21FrequencyMoverSpawner::SpawnLaneLabel(const FString& LabelText, const FVector& Location, const FColor& Color)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATextRenderActor* LabelActor = GetWorld()->SpawnActor<ATextRenderActor>(
		ATextRenderActor::StaticClass(),
		Location,
		FRotator(0.f, 180.f, 0.f),
		SpawnParams
	);

	if (!LabelActor)
	{
		return;
	}

	LabelActor->SetReplicates(true);

	if (UTextRenderComponent* Label = LabelActor->GetTextRender())
	{
		Label->SetHorizontalAlignment(EHTA_Center);
		Label->SetWorldSize(80.f);
		Label->SetTextRenderColor(Color);
		Label->SetText(FText::FromString(LabelText));
	}
}
