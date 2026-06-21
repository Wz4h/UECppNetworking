// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NetLesson17GameMode.generated.h"

UCLASS()
class UENET_API ANetLesson17GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANetLesson17GameMode();

	virtual void BeginPlay() override;
};
