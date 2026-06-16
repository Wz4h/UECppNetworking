// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson12/NetLesson12RelevancyActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "NetLessons/Lesson12/NetLesson12PlayerState.h"
#include "UObject/ConstructorHelpers.h"

ANetLesson12RelevancyActor::ANetLesson12RelevancyActor()
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
	if (Mode == ENetLesson12RelevancyMode::TeamOnly)
	{
		return TeamId != 0 && GetViewerTeamId(RealViewer, ViewTarget) == TeamId;
	}

	return Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
}

void ANetLesson12RelevancyActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetLesson12RelevancyActor, Mode);
	DOREPLIFETIME(ANetLesson12RelevancyActor, DemoName);
	DOREPLIFETIME(ANetLesson12RelevancyActor, RuleText);
	DOREPLIFETIME(ANetLesson12RelevancyActor, ColorIndex);
	DOREPLIFETIME(ANetLesson12RelevancyActor, DemoValue);
	DOREPLIFETIME(ANetLesson12RelevancyActor, TeamId);
}

void ANetLesson12RelevancyActor::ConfigureDemoActor(ENetLesson12RelevancyMode InMode, const FString& InName, const FString& InRuleText, int32 InColorIndex, int32 InTeamId)
{
	if (!HasAuthority())
	{
		return;
	}

	Mode = InMode;
	DemoName = InName;
	RuleText = InRuleText;
	ColorIndex = InColorIndex;
	TeamId = InTeamId;

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

void ANetLesson12RelevancyActor::SetTeamIdOnServer(int32 InTeamId)
{
	if (!HasAuthority())
	{
		return;
	}

	TeamId = InTeamId;
	UpdateVisual();
}

int32 ANetLesson12RelevancyActor::GetTeamId() const
{
	return TeamId;
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
	const FString TeamText = TeamId > 0 ? FString::Printf(TEXT("\nTeam=%d"), TeamId) : TEXT("");
	const FString Label = FString::Printf(
		TEXT("%s\n%s\n%s%s\nValue=%d"),
		*DemoName,
		*MachineText,
		*RuleText,
		*TeamText,
		DemoValue
	);

	DemoText->SetText(FText::FromString(Label));
}

int32 ANetLesson12RelevancyActor::GetViewerTeamId(const AActor* RealViewer, const AActor* ViewTarget) const
{
	const APawn* ViewerPawn = Cast<APawn>(ViewTarget);
	if (!ViewerPawn)
	{
		ViewerPawn = Cast<APawn>(RealViewer);
	}

	if (ViewerPawn)
	{
		if (const ANetLesson12PlayerState* LessonPlayerState = ViewerPawn->GetPlayerState<ANetLesson12PlayerState>())
		{
			return LessonPlayerState->GetTeamId();
		}
	}

	const AController* ViewerController = Cast<AController>(RealViewer);
	if (ViewerController)
	{
		if (const ANetLesson12PlayerState* LessonPlayerState = ViewerController->GetPlayerState<ANetLesson12PlayerState>())
		{
			return LessonPlayerState->GetTeamId();
		}
	}

	return 0;
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
