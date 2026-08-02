// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/InteractionDemoCharacter.h"

#include "Components/InputComponent.h"
#include "InputCoreTypes.h"
#include "Interaction/InteractionComponent.h"

AInteractionDemoCharacter::AInteractionDemoCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));
}

void AInteractionDemoCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bUpdateTargetEveryTick && InteractionComponent)
	{
		InteractionComponent->UpdateInteractionTarget();
	}
}

void AInteractionDemoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (!PlayerInputComponent)
	{
		return;
	}

	PlayerInputComponent->BindKey(EKeys::E, IE_Pressed, this, &ThisClass::HandleInteractPressed);
}

void AInteractionDemoCharacter::HandleInteractPressed()
{
	if (!InteractionComponent)
	{
		return;
	}

	InteractionComponent->UpdateInteractionTarget();
	InteractionComponent->TryInteract();
}
