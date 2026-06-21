// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetLesson13AlwaysActor.generated.h"

class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS()
class UENET_API ANetLesson13AlwaysActor : public AActor
{
	GENERATED_BODY()

public:
	ANetLesson13AlwaysActor();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void AddValueOnServer();
	void ToggleDistanceOnServer();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Network Lesson 13")
	UStaticMeshComponent* DemoMesh;

	UPROPERTY(VisibleAnywhere, Category = "Network Lesson 13")
	UTextRenderComponent* DemoText;

	UPROPERTY(ReplicatedUsing = OnRep_Value, VisibleAnywhere, Category = "Network Lesson 13")
	int32 Value = 1;

	UFUNCTION()
	void OnRep_Value();

	void UpdateVisual();
};
