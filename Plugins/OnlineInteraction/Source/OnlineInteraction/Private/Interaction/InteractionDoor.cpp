// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/InteractionDoor.h"

#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

AInteractionDoor::AInteractionDoor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	SetRootComponent(DoorMesh);
}

bool AInteractionDoor::CanInteract_Implementation(AActor* Interactor) const
{
	return true;
}

void AInteractionDoor::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority())
	{
		return;
	}

	bIsOpen = !bIsOpen;
	OnRep_IsOpen();
}

FInteractionInfo AInteractionDoor::GetInteractionInfo_Implementation() const
{
	FInteractionInfo Info;
	Info.InteractionText = bIsOpen ? FText::FromString(TEXT("Close Door")) : FText::FromString(TEXT("Open Door"));
	return Info;
}

void AInteractionDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInteractionDoor, bIsOpen);
}

void AInteractionDoor::OnRep_IsOpen()
{
	ApplyDoorRotation();

	UE_LOG(LogTemp, Log, TEXT("Door %s"), bIsOpen ? TEXT("Open") : TEXT("Close"));
}

void AInteractionDoor::ApplyDoorRotation()
{
	SetActorRotation(bIsOpen ? OpenRotation : ClosedRotation);
}
