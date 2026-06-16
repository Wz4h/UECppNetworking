// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NetLesson12Character.generated.h"

class UTextRenderComponent;

UCLASS()
class UENET_API ANetLesson12Character : public ACharacter
{
	GENERATED_BODY()

public:
	ANetLesson12Character();

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Network Lesson 12")
	UTextRenderComponent* LessonText;

	void RequestToggleDistanceActor();

	UFUNCTION(Server, Reliable)
	void ServerToggleDistanceActor();

	void UpdateLessonText();
};
