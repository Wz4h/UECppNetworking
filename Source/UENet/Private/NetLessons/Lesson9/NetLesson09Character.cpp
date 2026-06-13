// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson9/NetLesson09Character.h"

#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "InputCoreTypes.h"
#include "NetLessons/Lesson9/NetLesson09DoorActor.h"

ANetLesson09Character::ANetLesson09Character()
{
	bReplicates = true;
}

void ANetLesson09Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindKey(EKeys::Two, IE_Pressed, this, &ThisClass::TestCharacterEntryRPC);
}

void ANetLesson09Character::TestCharacterEntryRPC()
{
	for (TActorIterator<ANetLesson09DoorActor> It(GetWorld()); It; ++It)
	{
		Server_Interact(*It);
		ShowScreenMessage(TEXT("[Lesson9] Character sent Server_Interact request"), FColor::Yellow);
		return;
	}

	ShowScreenMessage(TEXT("[Lesson9] No Door found"), FColor::Red);
}

void ANetLesson09Character::Server_Interact_Implementation(ANetLesson09DoorActor* TargetDoor)
{
	//if (HasAuthority()) return;
	
	if (!IsValid(TargetDoor))
	{
		ShowScreenMessage(TEXT("[Lesson9] Server rejected invalid Door"), FColor::Red);
		return;
	}

	TargetDoor->OpenDoor();
	ShowScreenMessage(TEXT("[Lesson9] Server opened Door through Character RPC"), FColor::Green);
}

void ANetLesson09Character::ShowScreenMessage(const FString& Message, const FColor& Color) const
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, Color, Message);
	}
}
