// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NetLesson13Character.generated.h"

class UTextRenderComponent;

UCLASS()
class UENET_API ANetLesson13Character : public ACharacter
{
	GENERATED_BODY()

public:
	ANetLesson13Character();

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Network Lesson 13")
	UTextRenderComponent* LessonText;

	void RequestUpdateAlwaysActor();

	UFUNCTION(Server, Reliable)
	void ServerUpdateAlwaysActor();

	void UpdateLessonText();
};
