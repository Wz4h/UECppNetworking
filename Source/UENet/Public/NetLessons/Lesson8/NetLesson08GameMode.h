// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NetLesson08GameMode.generated.h"

UCLASS()
class UENET_API ANetLesson08GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANetLesson08GameMode();
	virtual void BeginPlay() override;
};
