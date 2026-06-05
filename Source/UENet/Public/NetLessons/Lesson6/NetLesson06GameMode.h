// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NetLesson06GameMode.generated.h"

UCLASS()
class UENET_API ANetLesson06GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANetLesson06GameMode();

	virtual void BeginPlay() override;
};
