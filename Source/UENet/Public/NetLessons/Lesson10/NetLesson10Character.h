// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NetLesson10Character.generated.h"

class UNetLesson10InteractionComponent;
class ANetLesson10WeaponActor;

UCLASS()
class UENET_API ANetLesson10Character : public ACharacter
{
	GENERATED_BODY()

public:
	ANetLesson10Character();

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(VisibleAnywhere)
	UNetLesson10InteractionComponent* InteractionComponent;

	UPROPERTY(Replicated)
	ANetLesson10WeaponActor* WeaponActor;

	void TestInteractionComponentRPC();
	void TestWeaponActorRPC();
};
