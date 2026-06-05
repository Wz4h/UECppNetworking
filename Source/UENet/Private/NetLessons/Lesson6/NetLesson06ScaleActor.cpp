// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson6/NetLesson06ScaleActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ANetLesson06ScaleActor::ANetLesson06ScaleActor()
{
	bReplicates = true;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	DemoBox = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DemoBox"));
	DemoBox->SetupAttachment(SceneRoot);
	DemoBox->SetRelativeLocation(FVector(0.f, 0.f, 80.f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		DemoBox->SetStaticMesh(CubeMesh.Object);
	}
}

void ANetLesson06ScaleActor::BeginPlay()
{
	Super::BeginPlay();

	UpdateBoxScale();
	PrintNetworkDebugInfo(TEXT("BeginPlay"));
}

void ANetLesson06ScaleActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetLesson06ScaleActor, ScaleValue);
}

void ANetLesson06ScaleActor::AddScaleValueOnServer()
{
	if (!HasAuthority())
	{
		PrintNetworkDebugInfo(TEXT("NoAuthority"));
		return;
	}

	ScaleValue += 0.25f;

	UpdateBoxScale();
	PrintNetworkDebugInfo(TEXT("AddScaleValueOnServer"));
}

void ANetLesson06ScaleActor::OnRep_ScaleValue()
{
	UpdateBoxScale();
	PrintNetworkDebugInfo(TEXT("OnRep_ScaleValue"));
}

void ANetLesson06ScaleActor::UpdateBoxScale()
{
	DemoBox->SetRelativeScale3D(FVector(ScaleValue));
}

void ANetLesson06ScaleActor::PrintNetworkDebugInfo(const FString& Prefix) const
{
	const FString MachineText = HasAuthority() ? TEXT("Server") : TEXT("Client");

	const FString Message = FString::Printf(
		TEXT("[Lesson6][%s] Machine=%s Actor=%s LocalRole=%s RemoteRole=%s ScaleValue=%.2f"),
		*Prefix,
		*MachineText,
		*GetName(),
		*NetRoleToString(GetLocalRole()),
		*NetRoleToString(GetRemoteRole()),
		ScaleValue
	);

	UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			3.f,
			FColor::Yellow,
			Message
		);
	}
}

FString ANetLesson06ScaleActor::NetRoleToString(ENetRole Role)
{
	switch (Role)
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
