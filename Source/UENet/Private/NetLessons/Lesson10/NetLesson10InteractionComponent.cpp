// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson10/NetLesson10InteractionComponent.h"

#include "Engine/Engine.h"
#include "NetLessons/Lesson10/NetLesson10DoorActor.h"

UNetLesson10InteractionComponent::UNetLesson10InteractionComponent()
{
	SetIsReplicatedByDefault(true);
}

void UNetLesson10InteractionComponent::InteractWithDoor(ANetLesson10DoorActor* TargetDoor)
{
	ShowScreenMessage(TEXT("[Lesson10] InteractionComponent requested Server_InteractWithDoor"), FColor::Yellow);
	Server_InteractWithDoor(TargetDoor);
}

void UNetLesson10InteractionComponent::Server_InteractWithDoor_Implementation(ANetLesson10DoorActor* TargetDoor)
{
	if (!IsValid(TargetDoor))
	{
		ShowScreenMessage(TEXT("[Lesson10] Server rejected invalid Door"), FColor::Red);
		return;
	}

	TargetDoor->OpenDoor();
	ShowScreenMessage(TEXT("[Lesson10] InteractionComponent Server RPC opened Door"), FColor::Green);
}

void UNetLesson10InteractionComponent::ShowScreenMessage(const FString& Message, const FColor& Color) const
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, Color, Message);
	}
}
