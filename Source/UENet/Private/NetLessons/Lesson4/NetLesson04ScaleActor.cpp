// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson4/NetLesson04ScaleActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ANetLesson04ScaleActor::ANetLesson04ScaleActor()
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

void ANetLesson04ScaleActor::BeginPlay()
{
	Super::BeginPlay();

	UpdateBoxScale();
	PrintNetworkDebugInfo(TEXT("BeginPlay"));
}

void ANetLesson04ScaleActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetLesson04ScaleActor, ReplicatedWidth);
}

void ANetLesson04ScaleActor::ChangeValuesOnServer()
{
	if (!HasAuthority())
	{
		PrintNetworkDebugInfo(TEXT("NoAuthority"));
		return;
	}

	ReplicatedWidth += 0.25f;
	LocalOnlyHeight += 0.25f;

	UpdateBoxScale();
	PrintNetworkDebugInfo(TEXT("ChangeValuesOnServer"));
}

void ANetLesson04ScaleActor::OnRep_ReplicatedWidth()
{
	UpdateBoxScale();
	PrintNetworkDebugInfo(TEXT("OnRep_ReplicatedWidth"));
}

void ANetLesson04ScaleActor::UpdateBoxScale()
{
	DemoBox->SetRelativeScale3D(FVector(1.f, ReplicatedWidth, LocalOnlyHeight));
}

void ANetLesson04ScaleActor::PrintNetworkDebugInfo(const FString& Prefix) const
{
	const FString MachineText = HasAuthority() ? TEXT("Server") : TEXT("Client");

	const FString Message = FString::Printf(
		TEXT("[Lesson4][%s] Machine=%s Actor=%s LocalRole=%s RemoteRole=%s ReplicatedWidth=%.2f LocalOnlyHeight=%.2f"),
		*Prefix,
		*MachineText,
		*GetName(),
		*NetRoleToString(GetLocalRole()),
		*NetRoleToString(GetRemoteRole()),
		ReplicatedWidth,
		LocalOnlyHeight
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

FString ANetLesson04ScaleActor::NetRoleToString(ENetRole Role)
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
