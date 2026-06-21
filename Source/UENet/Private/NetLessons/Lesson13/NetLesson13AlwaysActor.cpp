// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson13/NetLesson13AlwaysActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ANetLesson13AlwaysActor::ANetLesson13AlwaysActor()
{
	bReplicates = true;
	
	bAlwaysRelevant = true;
	
	SetReplicateMovement(true);
	
	SetNetCullDistanceSquared(1.f);
	
	SetNetUpdateFrequency(30.f);
	SetMinNetUpdateFrequency(30.f);

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

void ANetLesson13AlwaysActor::BeginPlay()
{
	Super::BeginPlay();

	UpdateVisual();
}

void ANetLesson13AlwaysActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetLesson13AlwaysActor, Value);
}

void ANetLesson13AlwaysActor::AddValueOnServer()
{
	if (!HasAuthority())
	{
		return;
	}

	Value++;
	UpdateVisual();
}

void ANetLesson13AlwaysActor::ToggleDistanceOnServer()
{
	if (!HasAuthority())
	{
		return;
	}

	const bool bIsFar = GetActorLocation().X > 1500.f;
	SetActorLocation(bIsFar ? FVector(520.f, 0.f, 80.f) : FVector(8000.f, 0.f, 80.f));
	AddValueOnServer();
	ForceNetUpdate();
}

void ANetLesson13AlwaysActor::OnRep_Value()
{
	UpdateVisual();
}

void ANetLesson13AlwaysActor::UpdateVisual()
{
	const float Size = 0.8f + Value * 0.15f;
	DemoMesh->SetRelativeScale3D(FVector(Size, Size, Size));

	const FString MachineText = HasAuthority() ? TEXT("Server World") : TEXT("Client World");
	const FString Label = FString::Printf(
		TEXT("AlwaysRelevant\n%s\nNear / Far\nStill Replicated\nValue=%d"),
		*MachineText,
		Value
	);
	DemoText->SetText(FText::FromString(Label));
}
