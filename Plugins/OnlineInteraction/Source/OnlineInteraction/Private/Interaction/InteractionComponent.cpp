// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/InteractionComponent.h"

#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Interaction/Interactable.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UInteractionComponent::UpdateInteractionTarget()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		CurrentTarget = nullptr;
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		CurrentTarget = nullptr;
		return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;

	APawn* PawnOwner = Cast<APawn>(Owner);
	if (PawnOwner && PawnOwner->GetController())
	{
		PawnOwner->GetController()->GetPlayerViewPoint(ViewLocation, ViewRotation);
	}
	else
	{
		ViewLocation = Owner->GetActorLocation();
		ViewRotation = Owner->GetActorRotation();
	}

	const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * TraceDistance;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(OnlineInteractionTrace), false);
	Params.AddIgnoredActor(Owner);

	const bool bHit = World->LineTraceSingleByChannel(
		Hit,
		ViewLocation,
		TraceEnd,
		TraceChannel,
		Params
	);

	if (bHit && IsValidInteractionTarget(Hit.GetActor()))
	{
		CurrentTarget = Hit.GetActor();
	}
	else
	{
		CurrentTarget = nullptr;
	}
}

void UInteractionComponent::TryInteract()
{
	if (!CurrentTarget)
	{
		return;
	}

	ServerInteract(CurrentTarget);
}

void UInteractionComponent::ServerInteract_Implementation(AActor* Target)
{
	if (!IsValidInteractionTarget(Target))
	{
		return;
	}

	if (!IsTargetInRange(Target))
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const bool bCanInteract = IInteractable::Execute_CanInteract(Target, Owner);
	if (!bCanInteract)
	{
		return;
	}

	IInteractable::Execute_Interact(Target, Owner);
	UE_LOG(LogTemp, Log, TEXT("Server Interact Success"));
}

bool UInteractionComponent::IsValidInteractionTarget(AActor* Target) const
{
	if (!Target)
	{
		return false;
	}

	if (!Target->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
	{
		return false;
	}

	return true;
}

bool UInteractionComponent::IsTargetInRange(AActor* Target) const
{
	AActor* Owner = GetOwner();
	if (!Owner || !Target)
	{
		return false;
	}

	const float DistanceSquared = FVector::DistSquared(
		Owner->GetActorLocation(),
		Target->GetActorLocation()
	);

	return DistanceSquared <= FMath::Square(TraceDistance);
}
