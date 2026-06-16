// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetLesson14OwnerActor.generated.h"

class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS()
class UENET_API ANetLesson14OwnerActor : public AActor
{
	GENERATED_BODY()

public:
	ANetLesson14OwnerActor();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void AddValueOnServer();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Network Lesson 14")
	UStaticMeshComponent* DemoMesh;

	UPROPERTY(VisibleAnywhere, Category = "Network Lesson 14")
	UTextRenderComponent* DemoText;

	UPROPERTY(ReplicatedUsing = OnRep_Value, VisibleAnywhere, Category = "Network Lesson 14")
	int32 Value = 1;

	UFUNCTION()
	void OnRep_Value();

	void UpdateVisual();
};
