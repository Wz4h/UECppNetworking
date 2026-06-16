// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NetLesson14GameMode.generated.h"

class APlayerController;

UCLASS()
class UENET_API ANetLesson14GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANetLesson14GameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;

protected:
	int32 PrivateActorIndex = 0;
};
