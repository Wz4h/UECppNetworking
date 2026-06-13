// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson8/NetLesson08DoorActor.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

ANetLesson08DoorActor::ANetLesson08DoorActor()
{
	bReplicates = true;

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	SetRootComponent(DoorMesh);
	DoorMesh->SetRelativeScale3D(FVector(0.2f, 1.4f, 2.f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		DoorMesh->SetStaticMesh(CubeMesh.Object);
	}
}

void ANetLesson08DoorActor::Server_TestDoorRPC_Implementation()
{
	if (HasAuthority())
	{
		
	}
	DoorMesh->SetRelativeScale3D(FVector(0.2f, 0.3f, 2.f));
	ShowScreenMessage(TEXT("[Lesson8] Door Server RPC EXECUTED on server"), FColor::Green);
}

void ANetLesson08DoorActor::ShowScreenMessage(const FString& Message, const FColor& Color) const
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, Color, Message);
	}
}
