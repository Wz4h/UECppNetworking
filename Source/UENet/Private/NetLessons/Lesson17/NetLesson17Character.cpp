// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson17/NetLesson17Character.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ANetLesson17Character::ANetLesson17Character()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	DemoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DemoMesh"));
	DemoMesh->SetupAttachment(SceneRoot);
	DemoMesh->SetRelativeLocation(FVector(0.f, 0.f, 60.f));
	DemoMesh->SetRelativeScale3D(FVector(1.f, 1.f, 1.2f));

	DebugText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("DebugText"));
	DebugText->SetupAttachment(SceneRoot);
	DebugText->SetRelativeLocation(FVector(0.f, 0.f, 190.f));
	DebugText->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	DebugText->SetHorizontalAlignment(EHTA_Center);
	DebugText->SetWorldSize(20.f);
	DebugText->SetTextRenderColor(FColor::White);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SceneRoot);
	Camera->SetRelativeLocation(FVector(-650.f, 0.f, 420.f));
	Camera->SetRelativeRotation(FRotator(-35.f, 0.f, 0.f));
	Camera->SetAutoActivate(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		DemoMesh->SetStaticMesh(CubeMesh.Object);
	}
}

void ANetLesson17Character::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SetActorLocation(FVector(0.f, 0.f, 100.f));
		SetServerScale(1.f);
	}

	LocalPredictedScale = ServerScale;
	ApplyScale(LocalPredictedScale);
	UpdateDebugText();
}

void ANetLesson17Character::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateDebugText();
}

void ANetLesson17Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetLesson17Character, ServerScale);
}

void ANetLesson17Character::PredictGrowLocally()
{
	LocalPredictedScale += ClientPredictionScaleStep;
	ApplyScale(LocalPredictedScale);
	UpdateDebugText();
	ShowEventMessage(TEXT("Client Prediction Grow"), FColor::Cyan);
}

void ANetLesson17Character::RequestGrowFromServer()
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(DelayedCorrectionTimerHandle);

	PendingServerScale = ServerScale + ServerAuthorityScaleStep;
	LocalPredictedScale = PendingServerScale;
	ApplyScale(PendingServerScale);
	UpdateDebugText();
	ShowEventMessage(TEXT("Server Grow Now, Replicate After 1s"), FColor::Green);

	GetWorldTimerManager().SetTimer(DelayedCorrectionTimerHandle, this, &ThisClass::ReplicateDelayedServerScale, ServerCorrectionDelaySeconds, false);
}

void ANetLesson17Character::RequestResetFromServer()
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(DelayedCorrectionTimerHandle);
	PendingServerScale = 1.f;
	SetServerScale(1.f);
	ShowEventMessage(TEXT("Server Reset Scale"), FColor::Green);
}

UCameraComponent* ANetLesson17Character::GetDemoCamera() const
{
	return Camera;
}

void ANetLesson17Character::OnRep_ServerScale()
{
	LocalPredictedScale = ServerScale;
	ApplyScale(ServerScale);
	UpdateDebugText();
	ShowEventMessage(TEXT("Client Receive Server Scale"), FColor::Cyan);
}

void ANetLesson17Character::ReplicateDelayedServerScale()
{
	SetServerScale(PendingServerScale);
	ShowEventMessage(TEXT("Server Replicate Correction Scale After 1s"), FColor::Green);
}

void ANetLesson17Character::ApplyScale(float NewScale)
{
	DemoMesh->SetRelativeScale3D(FVector(NewScale, NewScale, NewScale * 1.2f));
}

void ANetLesson17Character::SetServerScale(float NewScale)
{
	if (!HasAuthority())
	{
		return;
	}

	ServerScale = NewScale;
	LocalPredictedScale = ServerScale;
	ApplyScale(ServerScale);
	ForceNetUpdate();
	UpdateDebugText();
}

void ANetLesson17Character::UpdateDebugText()
{
	const FString MachineText = HasAuthority() ? TEXT("Server") : TEXT("Client");
	const FString Text = FString::Printf(
		TEXT("Lesson17 Prediction + Correction\nMachine=%s LocalRole=%s RemoteRole=%s\nActor Location=%s\nLocal Predicted Scale=%.2f\nServer Scale=%.2f Pending Server Scale=%.2f\nClient Step=%.2f Server Step=%.2f Correction Delay=%.0f ms\nW: Client Grow -> 1s -> Server Grow -> 1s -> Client Correct"),
		*MachineText,
		*NetRoleToString(GetLocalRole()),
		*NetRoleToString(GetRemoteRole()),
		*GetActorLocation().ToCompactString(),
		LocalPredictedScale,
		ServerScale,
		PendingServerScale,
		ClientPredictionScaleStep,
		ServerAuthorityScaleStep,
		ServerCorrectionDelaySeconds * 1000.f
	);

	DebugText->SetText(FText::FromString(Text));
}

void ANetLesson17Character::ShowEventMessage(const FString& Message, const FColor& Color) const
{
	const FString FullMessage = FString::Printf(TEXT("[Lesson17] %s"), *Message);
	UE_LOG(LogTemp, Warning, TEXT("%s"), *FullMessage);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.5f, Color, FullMessage);
	}
}

FString ANetLesson17Character::NetRoleToString(ENetRole InRole) const
{
	switch (InRole)
	{
	case ROLE_None:
		return TEXT("None");
	case ROLE_SimulatedProxy:
		return TEXT("SimulatedProxy");
	case ROLE_AutonomousProxy:
		return TEXT("AutonomousProxy");
	case ROLE_Authority:
		return TEXT("Authority");
	default:
		return TEXT("Unknown");
	}
}
