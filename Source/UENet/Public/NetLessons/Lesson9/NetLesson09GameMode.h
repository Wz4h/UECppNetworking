// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NetLesson09GameMode.generated.h"

UCLASS()
class UENET_API ANetLesson09GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANetLesson09GameMode();
	virtual void BeginPlay() override;
};
