// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetLesson07Visualizer.generated.h"

class UTextRenderComponent;

UCLASS()
class UENET_API ANetLesson07Visualizer : public AActor
{
	GENERATED_BODY()

public:
	ANetLesson07Visualizer();

	void UpdateFrameworkView(bool bIsServerWorld, bool bHasGameMode, bool bHasGameState,
		int32 PlayerControllerCount, int32 PlayerStateCount, int32 PawnCount);

protected:
	UPROPERTY(VisibleAnywhere)
	UTextRenderComponent* TitleText;

	UPROPERTY(VisibleAnywhere)
	UTextRenderComponent* GameModeText;

	UPROPERTY(VisibleAnywhere)
	UTextRenderComponent* GameStateText;

	UPROPERTY(VisibleAnywhere)
	UTextRenderComponent* PlayerControllerText;

	UPROPERTY(VisibleAnywhere)
	UTextRenderComponent* PlayerStateText;

	UPROPERTY(VisibleAnywhere)
	UTextRenderComponent* PawnText;

	void SetPresenceText(UTextRenderComponent* TextComponent, const TCHAR* Label, bool bPresent);
	void SetCountText(UTextRenderComponent* TextComponent, const TCHAR* Label, int32 Count);
};
