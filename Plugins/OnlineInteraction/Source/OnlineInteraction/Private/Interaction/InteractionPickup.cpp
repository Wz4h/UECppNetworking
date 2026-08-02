// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/InteractionPickup.h"

#include "Components/StaticMeshComponent.h"

AInteractionPickup::AInteractionPickup()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	SetRootComponent(PickupMesh);
}

bool AInteractionPickup::CanInteract_Implementation(AActor* Interactor) const
{
	return true;
}

void AInteractionPickup::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Pickup Destroy"));
	Destroy();
}

FInteractionInfo AInteractionPickup::GetInteractionInfo_Implementation() const
{
	FInteractionInfo Info;
	Info.InteractionText = FText::FromString(TEXT("Pickup"));
	return Info;
}
