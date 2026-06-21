// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson12/NetLesson12RelevancyActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ANetLesson12RelevancyActor::ANetLesson12RelevancyActor()
{
	bReplicates = true;
	SetReplicateMovement(true);
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
	DemoText->SetWorldSize(18.f);
	DemoText->SetTextRenderColor(FColor::White);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		DemoMesh->SetStaticMesh(CubeMesh.Object);
	}
}

void ANetLesson12RelevancyActor::BeginPlay()
{
	Super::BeginPlay();

	UpdateVisual();
}

bool ANetLesson12RelevancyActor::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	return FVector::DistSquared(SrcLocation, GetActorLocation()) <= GetNetCullDistanceSquared();
}

void ANetLesson12RelevancyActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetLesson12RelevancyActor, DemoName);
	DOREPLIFETIME(ANetLesson12RelevancyActor, RuleText);
	DOREPLIFETIME(ANetLesson12RelevancyActor, ColorIndex);
	DOREPLIFETIME(ANetLesson12RelevancyActor, DemoValue);
}

void ANetLesson12RelevancyActor::ConfigureDemoActor(const FString& InName, const FString& InRuleText, int32 InColorIndex)
{
	if (!HasAuthority())
	{
		return;
	}

	DemoName = InName;
	RuleText = InRuleText;
	ColorIndex = InColorIndex;

	UpdateVisual();
}

void ANetLesson12RelevancyActor::AddValueOnServer()
{
	if (!HasAuthority())
	{
		return;
	}

	DemoValue++;
	UpdateVisual();
}

void ANetLesson12RelevancyActor::OnRep_Visual()
{
	UpdateVisual();
}

void ANetLesson12RelevancyActor::UpdateVisual()
{
	const float Size = 0.75f + DemoValue * 0.12f;
	DemoMesh->SetRelativeScale3D(FVector(Size, Size, Size));

	if (UMaterialInstanceDynamic* DynamicMaterial = DemoMesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		DynamicMaterial->SetVectorParameterValue(TEXT("Color"), GetDemoColor());
	}

	const FString MachineText = HasAuthority() ? TEXT("Server World") : TEXT("Client World");
	const FString Label = FString::Printf(
		TEXT("%s\n%s\n%s\nValue=%d"),
		*DemoName,
		*MachineText,
		*RuleText,
		DemoValue
	);

	DemoText->SetText(FText::FromString(Label));
}

FLinearColor ANetLesson12RelevancyActor::GetDemoColor() const
{
	switch (ColorIndex)
	{
	case 1:
		return FLinearColor(0.1f, 0.45f, 1.f);

	case 2:
		return FLinearColor(0.1f, 0.9f, 0.35f);

	case 3:
		return FLinearColor(1.f, 0.75f, 0.1f);

	case 4:
		return FLinearColor(1.f, 0.2f, 0.25f);

	default:
		return FLinearColor(0.8f, 0.8f, 0.8f);
	}
}
