// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson17/NetLesson17PlayerController.h"

#include "Camera/CameraComponent.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "InputCoreTypes.h"
#include "NetLessons/Lesson17/NetLesson17Character.h"
#include "TimerManager.h"

ANetLesson17PlayerController::ANetLesson17PlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ANetLesson17PlayerController::BeginPlay()
{
	Super::BeginPlay();

	TryActivateLessonCamera();
}

void ANetLesson17PlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bLessonCameraActivated)
	{
		TryActivateLessonCamera();
	}
}

void ANetLesson17PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindKey(EKeys::W, IE_Pressed, this, &ThisClass::HandleGrow);
	InputComponent->BindKey(EKeys::R, IE_Pressed, this, &ThisClass::HandleReset);
}

void ANetLesson17PlayerController::HandleGrow()
{
	TryActivateLessonCamera();

	if (ANetLesson17Character* LessonActor = FindLessonActor())
	{
		// 客户端先在本地变大，用来演示 Prediction。
		if (!HasAuthority())
		{
			LessonActor->PredictGrowLocally();
		}
	}

	if (HasAuthority())
	{
		ServerRequestGrow_Implementation();
		return;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Yellow, TEXT("[Lesson17] Client Wait 1s Then Send Grow RPC"));
	}

	GetWorldTimerManager().ClearTimer(DelayedGrowRpcTimerHandle);
	GetWorldTimerManager().SetTimer(DelayedGrowRpcTimerHandle, this, &ThisClass::SendDelayedGrowRpc, ClientToServerDelaySeconds, false);
}

void ANetLesson17PlayerController::HandleReset()
{
	TryActivateLessonCamera();

	if (HasAuthority())
	{
		ServerRequestReset_Implementation();
		return;
	}

	ServerRequestReset();
}

void ANetLesson17PlayerController::SendDelayedGrowRpc()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Yellow, TEXT("[Lesson17] Client Send Grow RPC After 1s"));
	}

	ServerRequestGrow();
}

void ANetLesson17PlayerController::TryActivateLessonCamera()
{
	if (bLessonCameraActivated)
	{
		return;
	}

	ANetLesson17Character* LessonActor = FindLessonActor();
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

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Cyan, TEXT("[Lesson17] Camera Target = Lesson17 Actor"));
	}
}

ANetLesson17Character* ANetLesson17PlayerController::FindLessonActor() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<ANetLesson17Character> It(World); It; ++It)
	{
		return *It;
	}

	return nullptr;
}

void ANetLesson17PlayerController::ServerRequestGrow_Implementation()
{
	if (ANetLesson17Character* LessonActor = FindLessonActor())
	{
		LessonActor->RequestGrowFromServer();
	}
}

void ANetLesson17PlayerController::ServerRequestReset_Implementation()
{
	if (ANetLesson17Character* LessonActor = FindLessonActor())
	{
		LessonActor->RequestResetFromServer();
	}
}
