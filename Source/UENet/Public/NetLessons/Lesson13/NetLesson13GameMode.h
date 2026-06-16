// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NetLesson13GameMode.generated.h"

UCLASS()
class UENET_API ANetLesson13GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANetLesson13GameMode();

	virtual void BeginPlay() override;
};
