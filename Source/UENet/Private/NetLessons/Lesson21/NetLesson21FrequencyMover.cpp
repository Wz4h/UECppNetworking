// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson21/NetLesson21FrequencyMover.h"

#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ANetLesson21FrequencyMover::ANetLesson21FrequencyMover()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMesh.Object);
	}

	ApplyFrequencyMode();
	UpdateDebugVisual();
}

void ANetLesson21FrequencyMover::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetActorLocation();
	ApplyFrequencyMode();
	UpdateDebugVisual();
}

void ANetLesson21FrequencyMover::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetLesson21FrequencyMover, FrequencyMode);
}

void ANetLesson21FrequencyMover::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority())
	{
		MoveActor(DeltaSeconds);
	}
}

void ANetLesson21FrequencyMover::SetFrequencyMode(ENetLesson21FrequencyMode NewMode)
{
	FrequencyMode = NewMode;
	ApplyFrequencyMode();
	UpdateDebugVisual();
}

void ANetLesson21FrequencyMover::ConfigureDemo(ENetLesson21FrequencyMode NewMode, float NewMoveDistance, float NewMoveSpeed, float NewPhaseOffset)
{
	FrequencyMode = NewMode;
	MoveDistance = NewMoveDistance;
	MoveSpeed = NewMoveSpeed;
	PhaseOffset = NewPhaseOffset;

	ApplyFrequencyMode();
	UpdateDebugVisual();
}

void ANetLesson21FrequencyMover::ApplyFrequencyMode()
{
	if (FrequencyMode == ENetLesson21FrequencyMode::HighFrequency)
	{
		SetNetUpdateFrequency(100.f);
		SetMinNetUpdateFrequency(30.f);
	}
	else
	{
		SetNetUpdateFrequency(0.5f);
		SetMinNetUpdateFrequency(0.25f);
	}
}

void ANetLesson21FrequencyMover::MoveActor(float DeltaSeconds)
{
	RunningTime += DeltaSeconds;

	const float SafeMoveSpeed = FMath::Max(1.f, MoveSpeed);
	const float SafeMoveDistance = FMath::Max(1.f, MoveDistance);
	const float Alpha = (FMath::Sin(((RunningTime + PhaseOffset) * SafeMoveSpeed / SafeMoveDistance) * PI) + 1.f) * 0.5f;
	const FVector NewLocation = StartLocation + FVector(FMath::Lerp(-SafeMoveDistance, SafeMoveDistance, Alpha), 0.f, 0.f);

	SetActorLocation(NewLocation);
}

void ANetLesson21FrequencyMover::UpdateDebugVisual()
{
	if (!MeshComponent)
	{
		return;
	}

	const bool bHighFrequency = FrequencyMode == ENetLesson21FrequencyMode::HighFrequency;
	MeshComponent->SetRelativeScale3D(bHighFrequency ? FVector(0.7f) : FVector(0.45f));

	if (UMaterialInstanceDynamic* DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0))
	{
		const FLinearColor Color = bHighFrequency
			? FLinearColor(0.05f, 0.8f, 0.25f)
			: FLinearColor(1.f, 0.12f, 0.05f);
		DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
	}
}

void ANetLesson21FrequencyMover::OnRep_FrequencyMode()
{
	UpdateDebugVisual();
}
