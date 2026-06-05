// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson7/NetLesson07PlayerController.h"

#include "EngineUtils.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "NetLessons/Lesson7/NetLesson07Visualizer.h"

void ANetLesson07PlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (!IsLocalController())
	{
		return;
	}

	EnsureVisualizer();
	UpdateVisualizer();
}

void ANetLesson07PlayerController::EnsureVisualizer()
{
	if (Visualizer)
	{
		return;
	}

	Visualizer = GetWorld()->SpawnActor<ANetLesson07Visualizer>();
}

void ANetLesson07PlayerController::UpdateVisualizer()
{
	if (!Visualizer)
	{
		return;
	}

	const FVector ViewLocation = PlayerCameraManager ? PlayerCameraManager->GetCameraLocation() : GetFocalLocation();
	const FRotator ViewRotation = PlayerCameraManager ? PlayerCameraManager->GetCameraRotation() : GetControlRotation();
	const FVector Forward = ViewRotation.Vector();

	Visualizer->SetActorLocation(ViewLocation + Forward * 500.f);
	Visualizer->SetActorRotation(FRotator(0.f, ViewRotation.Yaw + 180.f, 0.f));

	int32 PlayerControllerCount = 0;
	int32 PlayerStateCount = 0;
	int32 PawnCount = 0;

	for (TActorIterator<APlayerController> It(GetWorld()); It; ++It)
	{
		++PlayerControllerCount;
	}

	for (TActorIterator<APlayerState> It(GetWorld()); It; ++It)
	{
		++PlayerStateCount;
	}

	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		++PawnCount;
	}

	Visualizer->UpdateFrameworkView(
		HasAuthority(),
		GetWorld()->GetAuthGameMode() != nullptr,
		GetWorld()->GetGameState() != nullptr,
		PlayerControllerCount,
		PlayerStateCount,
		PawnCount
	);
}
