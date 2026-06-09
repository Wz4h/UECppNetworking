// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson10/NetLesson10DoorActor.h"

#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ANetLesson10DoorActor::ANetLesson10DoorActor()
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

void ANetLesson10DoorActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetLesson10DoorActor, bIsOpen);
}

void ANetLesson10DoorActor::OpenDoor()
{
	if (!HasAuthority())
	{
		return;
	}

	bIsOpen = true;
	UpdateDoorVisual();
}

void ANetLesson10DoorActor::OnRep_IsOpen()
{
	UpdateDoorVisual();
}

void ANetLesson10DoorActor::UpdateDoorVisual()
{
	DoorMesh->SetRelativeScale3D(bIsOpen ? FVector(0.2f, 0.3f, 2.f) : FVector(0.2f, 1.4f, 2.f));
}
