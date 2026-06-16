// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson11/NetLesson11FrameworkActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ANetLesson11FrameworkActor::ANetLesson11FrameworkActor()
{
	bReplicates = true;
	SetReplicateMovement(true);

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	DemoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DemoMesh"));
	DemoMesh->SetupAttachment(SceneRoot);
	DemoMesh->SetRelativeLocation(FVector(0.f, 0.f, 80.f));

	DemoText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("DemoText"));
	DemoText->SetupAttachment(SceneRoot);
	DemoText->SetRelativeLocation(FVector(0.f, 0.f, 210.f));
	DemoText->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	DemoText->SetHorizontalAlignment(EHTA_Center);
	DemoText->SetWorldSize(24.f);
	DemoText->SetTextRenderColor(FColor::White);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		DemoMesh->SetStaticMesh(CubeMesh.Object);
	}
}

void ANetLesson11FrameworkActor::BeginPlay()
{
	Super::BeginPlay();

	UpdateVisual();
}

void ANetLesson11FrameworkActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetLesson11FrameworkActor, DemoName);
	DOREPLIFETIME(ANetLesson11FrameworkActor, ChannelHint);
	DOREPLIFETIME(ANetLesson11FrameworkActor, ColorIndex);
	DOREPLIFETIME(ANetLesson11FrameworkActor, DemoValue);
}

void ANetLesson11FrameworkActor::ConfigureDemoActor(const FString& InDemoName, int32 InColorIndex, const FString& InChannelHint)
{
	if (!HasAuthority())
	{
		return;
	}

	DemoName = InDemoName;
	ColorIndex = InColorIndex;
	ChannelHint = InChannelHint;
	UpdateVisual();
}

void ANetLesson11FrameworkActor::AddDemoValueOnServer()
{
	if (!HasAuthority())
	{
		return;
	}

	DemoValue++;
	UpdateVisual();
}

void ANetLesson11FrameworkActor::SetServerLocation(const FVector& NewLocation)
{
	if (!HasAuthority())
	{
		return;
	}

	SetActorLocation(NewLocation);
}

void ANetLesson11FrameworkActor::OnRep_DemoValue()
{
	UpdateVisual();
}

void ANetLesson11FrameworkActor::UpdateVisual()
{
	const float Size = 0.75f + DemoValue * 0.2f;
	DemoMesh->SetRelativeScale3D(FVector(Size, Size, Size));

	if (UMaterialInstanceDynamic* DynamicMaterial = DemoMesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		DynamicMaterial->SetVectorParameterValue(TEXT("Color"), GetDemoColor());
	}

	const FString MachineText = HasAuthority() ? TEXT("Server World") : TEXT("Client World");
	const FString Label = FString::Printf(
		TEXT("%s\n%s\nValue=%d\n%s"),
		*DemoName,
		*MachineText,
		DemoValue,
		*ChannelHint
	);

	DemoText->SetText(FText::FromString(Label));
}

FLinearColor ANetLesson11FrameworkActor::GetDemoColor() const
{
	switch (ColorIndex)
	{
	case 1:
		return FLinearColor(0.1f, 0.45f, 1.f);

	case 2:
		return FLinearColor(0.1f, 0.9f, 0.35f);

	case 3:
		return FLinearColor(1.f, 0.75f, 0.1f);

	default:
		return FLinearColor(0.8f, 0.8f, 0.8f);
	}
}
