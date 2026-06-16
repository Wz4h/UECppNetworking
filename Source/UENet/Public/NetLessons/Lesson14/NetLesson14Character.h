// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NetLesson14Character.generated.h"

class UTextRenderComponent;

UCLASS()
class UENET_API ANetLesson14Character : public ACharacter
{
	GENERATED_BODY()

public:
	ANetLesson14Character();

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Network Lesson 14")
	UTextRenderComponent* LessonText;

	void RequestUpdateOwnerActor();

	UFUNCTION(Server, Reliable)
	void ServerUpdateOwnerActor();

	void UpdateLessonText();
};
