// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NetLesson09Character.generated.h"

class ANetLesson09DoorActor;

UCLASS()
class UENET_API ANetLesson09Character : public ACharacter
{
	GENERATED_BODY()

public:
	ANetLesson09Character();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	void TestCharacterEntryRPC();

	UFUNCTION(Server, Reliable)
	void Server_Interact(ANetLesson09DoorActor* TargetDoor);

	void ShowScreenMessage(const FString& Message, const FColor& Color) const;
};
