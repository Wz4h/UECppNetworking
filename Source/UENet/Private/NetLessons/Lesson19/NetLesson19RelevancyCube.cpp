// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson19/NetLesson19RelevancyCube.h"

#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ANetLesson19RelevancyCube::ANetLesson19RelevancyCube()
{
	bReplicates = true;
	SetReplicateMovement(false);
	SetNetUpdateFrequency(10.f);
	SetMinNetUpdateFrequency(2.f);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);
	MeshComponent->SetIsReplicated(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMesh.Object);
	}
}
