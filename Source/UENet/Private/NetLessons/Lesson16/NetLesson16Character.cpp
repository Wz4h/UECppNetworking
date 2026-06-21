// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson16/NetLesson16Character.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ANetLesson16Character::ANetLesson16Character()
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

void ANetLesson16Character::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SetServerLocation(FVector(0.f, 0.f, 100.f));
	}

	UpdateDebugText();
}

void ANetLesson16Character::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateDebugText();
}

void ANetLesson16Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetLesson16Character, ServerLocation);
	DOREPLIFETIME(ANetLesson16Character, ServerScale);
}

void ANetLesson16Character::RequestMoveFromServer()
{
	if (!HasAuthority())
	{
		return;
	}

	ShowEventMessage(TEXT("Server RPC Received After Client Delay"), FColor::Orange);
	GetWorldTimerManager().ClearTimer(DelayedMoveTimerHandle);

	const FVector NewLocation = GetActorLocation() + FVector(MoveDistance, 0.f, 0.f);
	const float NewScale = ServerScale + ScaleStep;

	SetActorLocation(NewLocation);
	DemoMesh->SetRelativeScale3D(FVector(NewScale, NewScale, NewScale * 1.2f));
	PendingReplicatedLocation = NewLocation;
	PendingReplicatedScale = NewScale;
	UpdateDebugText();
	ShowEventMessage(TEXT("Server Move And Scale Immediately"), FColor::Green);

	GetWorldTimerManager().SetTimer(DelayedMoveTimerHandle, this, &ThisClass::ReplicateDelayedServerLocation, DownstreamDelaySeconds, false);
}

void ANetLesson16Character::RequestResetFromServer()
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(DelayedMoveTimerHandle);
	PendingReplicatedLocation = FVector(0.f, 0.f, 100.f);
	PendingReplicatedScale = 1.f;
	SetServerLocation(FVector(0.f, 0.f, 100.f));
	SetServerScale(1.f);
	ShowEventMessage(TEXT("Server Reset Position"), FColor::Green);
}

UCameraComponent* ANetLesson16Character::GetDemoCamera() const
{
	return Camera;
}

void ANetLesson16Character::OnRep_ServerLocation()
{
	ApplyServerLocation();
	ShowEventMessage(TEXT("Client Receive Replicated Position"), FColor::Cyan);
}

void ANetLesson16Character::OnRep_ServerScale()
{
	ApplyServerScale();
	ShowEventMessage(TEXT("Client Receive Replicated Scale"), FColor::Cyan);
}

void ANetLesson16Character::ReplicateDelayedServerLocation()
{
	SetServerLocation(PendingReplicatedLocation);
	SetServerScale(PendingReplicatedScale);
	ShowEventMessage(TEXT("Server Replicate Position And Scale After 1s"), FColor::Green);
}

void ANetLesson16Character::ApplyServerLocation()
{
	SetActorLocation(ServerLocation);
	UpdateDebugText();
}

void ANetLesson16Character::ApplyServerScale()
{
	DemoMesh->SetRelativeScale3D(FVector(ServerScale, ServerScale, ServerScale * 1.2f));
	UpdateDebugText();
}

void ANetLesson16Character::SetServerLocation(const FVector& NewLocation)
{
	if (!HasAuthority())
	{
		return;
	}

	ServerLocation = NewLocation;
	SetActorLocation(ServerLocation);
	ForceNetUpdate();
	UpdateDebugText();
}

void ANetLesson16Character::SetServerScale(float NewScale)
{
	if (!HasAuthority())
	{
		return;
	}

	ServerScale = NewScale;
	ApplyServerScale();
	ForceNetUpdate();
}

void ANetLesson16Character::UpdateDebugText()
{
	const FString MachineText = HasAuthority() ? TEXT("Server") : TEXT("Client");
	const FString Text = FString::Printf(
		TEXT("Lesson16 ServerOnly Network Delay\nMachine=%s LocalRole=%s RemoteRole=%s\nLocal Position=%s\nReplicated ServerLocation=%s\nServerScale=%.2f\nW: Client Wait 1s -> RPC -> Server Move/Scale -> Wait 1s -> Replicate\nDownDelay=%.0f ms MoveDistance=%.0f"),
		*MachineText,
		*NetRoleToString(GetLocalRole()),
		*NetRoleToString(GetRemoteRole()),
		*GetActorLocation().ToCompactString(),
		*ServerLocation.ToCompactString(),
		ServerScale,
		DownstreamDelaySeconds * 1000.f,
		MoveDistance
	);

	DebugText->SetText(FText::FromString(Text));
}

void ANetLesson16Character::ShowEventMessage(const FString& Message, const FColor& Color) const
{
	const FString FullMessage = FString::Printf(TEXT("[Lesson16] %s"), *Message);
	UE_LOG(LogTemp, Warning, TEXT("%s"), *FullMessage);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.5f, Color, FullMessage);
	}
}

FString ANetLesson16Character::NetRoleToString(ENetRole InRole) const
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
