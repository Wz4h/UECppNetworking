// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson20/NetLesson20DormancyDoor.h"

#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ANetLesson20DormancyDoor::ANetLesson20DormancyDoor()
{
	bReplicates = true;
	SetReplicateMovement(false);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);
	MeshComponent->SetRelativeScale3D(FVector(0.2f, 1.4f, 2.f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMesh.Object);
	}
}

void ANetLesson20DormancyDoor::BeginPlay()
{
	Super::BeginPlay();

	ClosedRotation = GetActorRotation();
	OpenRotation = ClosedRotation + FRotator(0.f, 90.f, 0.f);
	ApplyDoorVisualState();

	if (HasAuthority() && bUseDormancy)
	{
		SetNetDormancy(DORM_DormantAll);
	}
}

void ANetLesson20DormancyDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetLesson20DormancyDoor, bIsOpen);
}

void ANetLesson20DormancyDoor::OpenDoor()
{
	if (!HasAuthority() || bIsOpen)
	{
		return;
	}

	if (bUseDormancy)
	{
		FlushNetDormancy();
	}

	bIsOpen = true;
	OnRep_IsOpen();

	if (bUseDormancy)
	{
		ForceNetUpdate();
		SetNetDormancy(DORM_DormantAll);
	}
}

void ANetLesson20DormancyDoor::SetUseDormancy(bool bInUseDormancy)
{
	bUseDormancy = bInUseDormancy;

	if (!HasAuthority())
	{
		return;
	}

	SetNetDormancy(bUseDormancy ? DORM_DormantAll : DORM_Awake);
}

void ANetLesson20DormancyDoor::OnRep_IsOpen()
{
	ApplyDoorVisualState();
}

void ANetLesson20DormancyDoor::ApplyDoorVisualState()
{
	SetActorRotation(bIsOpen ? OpenRotation : ClosedRotation);
}
