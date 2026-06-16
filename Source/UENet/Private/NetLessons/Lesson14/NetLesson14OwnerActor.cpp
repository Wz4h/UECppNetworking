// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson14/NetLesson14OwnerActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ANetLesson14OwnerActor::ANetLesson14OwnerActor()
{
	bReplicates = true;
	bOnlyRelevantToOwner = true;
	SetReplicateMovement(true);

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	DemoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DemoMesh"));
	DemoMesh->SetupAttachment(SceneRoot);
	DemoMesh->SetRelativeLocation(FVector(0.f, 0.f, 80.f));

	DemoText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("DemoText"));
	DemoText->SetupAttachment(SceneRoot);
	DemoText->SetRelativeLocation(FVector(0.f, 0.f, 220.f));
	DemoText->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	DemoText->SetHorizontalAlignment(EHTA_Center);
	DemoText->SetWorldSize(20.f);
	DemoText->SetTextRenderColor(FColor::White);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		DemoMesh->SetStaticMesh(CubeMesh.Object);
	}
}

void ANetLesson14OwnerActor::BeginPlay()
{
	Super::BeginPlay();

	UpdateVisual();
}

void ANetLesson14OwnerActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetLesson14OwnerActor, Value);
}

void ANetLesson14OwnerActor::AddValueOnServer()
{
	if (!HasAuthority())
	{
		return;
	}

	Value++;
	UpdateVisual();
}

void ANetLesson14OwnerActor::OnRep_Value()
{
	UpdateVisual();
}

void ANetLesson14OwnerActor::UpdateVisual()
{
	const float Size = 0.8f + Value * 0.15f;
	DemoMesh->SetRelativeScale3D(FVector(Size, Size, Size));

	const FString MachineText = HasAuthority() ? TEXT("Server World") : TEXT("Owner Client World");
	const FString Label = FString::Printf(
		TEXT("OwnerOnly Actor\n%s\nOnly Owner Sees This\nValue=%d"),
		*MachineText,
		Value
	);
	DemoText->SetText(FText::FromString(Label));
}
