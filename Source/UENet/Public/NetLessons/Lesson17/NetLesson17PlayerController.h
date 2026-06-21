// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NetLesson17PlayerController.generated.h"

class ANetLesson17Character;

UCLASS()
class UENET_API ANetLesson17PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ANetLesson17PlayerController();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupInputComponent() override;

protected:
	// Actor 不是 Pawn，所以由 PlayerController 主动切到它的相机。
	bool bLessonCameraActivated = false;

	// 客户端预测后等待一段时间，再把请求发给服务器。
	float ClientToServerDelaySeconds = 1.f;

	FTimerHandle DelayedGrowRpcTimerHandle;

	void HandleGrow();
	void HandleReset();
	void SendDelayedGrowRpc();
	void TryActivateLessonCamera();
	ANetLesson17Character* FindLessonActor() const;

	UFUNCTION(Server, Reliable)
	void ServerRequestGrow();

	UFUNCTION(Server, Reliable)
	void ServerRequestReset();
};
