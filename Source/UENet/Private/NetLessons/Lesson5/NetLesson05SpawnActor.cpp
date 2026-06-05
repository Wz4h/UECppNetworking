// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson5/NetLesson05SpawnActor.h"

#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ANetLesson05SpawnActor::ANetLesson05SpawnActor()
{
	bReplicates = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMesh.Object);
	}

	MeshComponent->SetRelativeScale3D(FVector(0.5f));
}
