// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NetLesson15GameMode.generated.h"

class APlayerController;

UCLASS()
class UENET_API ANetLesson15GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANetLesson15GameMode();

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

protected:
	int32 NextTeamId = 1;
};
