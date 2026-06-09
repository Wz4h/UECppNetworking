// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson9/NetLesson09DoorActor.h"

#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ANetLesson09DoorActor::ANetLesson09DoorActor()
{
	bReplicates = true;

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	SetRootComponent(DoorMesh);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		DoorMesh->SetStaticMesh(CubeMesh.Object);
	}

	UpdateDoorVisual();
}

void ANetLesson09DoorActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetLesson09DoorActor, bIsOpen);
}

void ANetLesson09DoorActor::OpenDoor()
{
	if (!HasAuthority())
	{
		return;
	}

	bIsOpen = true;
	UpdateDoorVisual();
}

void ANetLesson09DoorActor::OnRep_IsOpen()
{
	UpdateDoorVisual();
}

void ANetLesson09DoorActor::UpdateDoorVisual()
{
	DoorMesh->SetRelativeScale3D(bIsOpen ? FVector(0.2f, 0.3f, 2.f) : FVector(0.2f, 1.4f, 2.f));
}
