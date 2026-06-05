// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NetLesson04GameMode.generated.h"

UCLASS()
class UENET_API ANetLesson04GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANetLesson04GameMode();

	virtual void BeginPlay() override;
};
