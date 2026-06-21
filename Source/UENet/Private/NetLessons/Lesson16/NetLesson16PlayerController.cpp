// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson16/NetLesson16PlayerController.h"

#include "Camera/CameraComponent.h"
#include "EngineUtils.h"
#include "InputCoreTypes.h"
#include "NetLessons/Lesson16/NetLesson16Character.h"
#include "TimerManager.h"

ANetLesson16PlayerController::ANetLesson16PlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ANetLesson16PlayerController::BeginPlay()
{
	Super::BeginPlay();

	TryActivateLessonCamera();
}

void ANetLesson16PlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bLessonCameraActivated)
	{
		TryActivateLessonCamera();
	}
}

void ANetLesson16PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindKey(EKeys::W, IE_Pressed, this, &ThisClass::HandleMoveForward);
	InputComponent->BindKey(EKeys::R, IE_Pressed, this, &ThisClass::HandleReset);
}

void ANetLesson16PlayerController::HandleMoveForward()
{
	TryActivateLessonCamera();

	if (HasAuthority())
	{
		ServerRequestMove_Implementation();
		return;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Yellow, TEXT("[Lesson16] Client Press W: Wait 1s Then Send RPC"));
	}

	GetWorldTimerManager().ClearTimer(DelayedMoveRpcTimerHandle);
	GetWorldTimerManager().SetTimer(DelayedMoveRpcTimerHandle, this, &ThisClass::SendDelayedMoveRpc, UpstreamDelaySeconds, false);
}

void ANetLesson16PlayerController::HandleReset()
{
	TryActivateLessonCamera();

	if (HasAuthority())
	{
		ServerRequestReset_Implementation();
		return;
	}

	ServerRequestReset();
}

void ANetLesson16PlayerController::SendDelayedMoveRpc()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Yellow, TEXT("[Lesson16] Client Send RPC After 1s"));
	}

	ServerRequestMove();
}

void ANetLesson16PlayerController::TryActivateLessonCamera()
{
	if (bLessonCameraActivated)
	{
		return;
	}

	ANetLesson16Character* LessonActor = FindLessonActor();
	if (!LessonActor)
	{
		return;
	}

	if (UCameraComponent* DemoCamera = LessonActor->GetDemoCamera())
	{
		DemoCamera->Activate(true);
	}

	SetViewTarget(LessonActor);
	bLessonCameraActivated = true;
}

ANetLesson16Character* ANetLesson16PlayerController::FindLessonActor() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<ANetLesson16Character> It(World); It; ++It)
	{
		return *It;
	}

	return nullptr;
}

void ANetLesson16PlayerController::ServerRequestMove_Implementation()
{
	if (ANetLesson16Character* LessonActor = FindLessonActor())
	{
		LessonActor->RequestMoveFromServer();
	}
}

void ANetLesson16PlayerController::ServerRequestReset_Implementation()
{
	if (ANetLesson16Character* LessonActor = FindLessonActor())
	{
		LessonActor->RequestResetFromServer();
	}
}
