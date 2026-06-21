// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NetLesson16PlayerController.generated.h"

class ANetLesson16Character;

UCLASS()
class UENET_API ANetLesson16PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ANetLesson16PlayerController();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupInputComponent() override;

protected:
	bool bLessonCameraActivated = false;
	float UpstreamDelaySeconds = 1.f;
	FTimerHandle DelayedMoveRpcTimerHandle;

	void HandleMoveForward();
	void HandleReset();
	void SendDelayedMoveRpc();
	void TryActivateLessonCamera();
	ANetLesson16Character* FindLessonActor() const;

	UFUNCTION(Server, Reliable)
	void ServerRequestMove();

	UFUNCTION(Server, Reliable)
	void ServerRequestReset();
};
