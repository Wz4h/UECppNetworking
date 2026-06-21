// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NetLesson15Character.generated.h"

class UTextRenderComponent;

UCLASS()
class UENET_API ANetLesson15Character : public ACharacter
{
	GENERATED_BODY()

public:
	ANetLesson15Character();

	virtual void BeginPlay() override;
	virtual void OnRep_PlayerState() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void UpdateLessonText();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Network Lesson 15")
	UTextRenderComponent* LessonText;

	void RequestToggleTeamActor();

	UFUNCTION(Server, Reliable)
	void ServerToggleTeamActor();
};
