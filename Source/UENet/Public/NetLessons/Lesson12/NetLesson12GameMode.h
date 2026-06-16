// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NetLesson12GameMode.generated.h"

class APlayerController;

UCLASS()
class UENET_API ANetLesson12GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANetLesson12GameMode();

	virtual void BeginPlay() override;

protected:
	void SpawnDistanceActor();
};
