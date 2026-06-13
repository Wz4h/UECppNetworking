// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NetLesson08Character.generated.h"

UCLASS()
class UENET_API ANetLesson08Character : public ACharacter
{
	GENERATED_BODY()

public:
	ANetLesson08Character();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	//server attck 
	//char1 char2 
	void TestDoorDirectRPC();
	void ShowScreenMessage(const FString& Message, const FColor& Color) const;
};
