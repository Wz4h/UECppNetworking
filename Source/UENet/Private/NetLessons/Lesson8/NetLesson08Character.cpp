// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson8/NetLesson08Character.h"

#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "InputCoreTypes.h"
#include "NetLessons/Lesson8/NetLesson08DoorActor.h"

ANetLesson08Character::ANetLesson08Character()
{
	bReplicates = true;
}

void ANetLesson08Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindKey(EKeys::One, IE_Pressed, this, &ThisClass::TestDoorDirectRPC);
}

void ANetLesson08Character::TestDoorDirectRPC()
{
	for (TActorIterator<ANetLesson08DoorActor> It(GetWorld()); It; ++It)
	{
		It->Server_TestDoorRPC();
		ShowScreenMessage(TEXT("[Lesson8] Client called Door Server RPC. Door has no client ownership, so server should not execute it."), FColor::Red);
		return;
	}

	ShowScreenMessage(TEXT("[Lesson8] No Door found"), FColor::Red);
}

void ANetLesson08Character::ShowScreenMessage(const FString& Message, const FColor& Color) const
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, Color, Message);
	}
}
