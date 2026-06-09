// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NetLesson10GameMode.generated.h"

UCLASS()
class UENET_API ANetLesson10GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANetLesson10GameMode();

	virtual void BeginPlay() override;
};
