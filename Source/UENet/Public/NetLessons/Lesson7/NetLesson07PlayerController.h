// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NetLesson07PlayerController.generated.h"

class ANetLesson07Visualizer;

UCLASS()
class UENET_API ANetLesson07PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void PlayerTick(float DeltaTime) override;

protected:
	UPROPERTY()
	ANetLesson07Visualizer* Visualizer;

	void EnsureVisualizer();
	void UpdateVisualizer();
};
