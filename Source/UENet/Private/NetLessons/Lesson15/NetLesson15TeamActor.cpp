// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson15/NetLesson15TeamActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "NetLessons/Lesson15/NetLesson15PlayerState.h"
#include "UObject/ConstructorHelpers.h"

ANetLesson15TeamActor::ANetLesson15TeamActor()
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
	DemoText->SetWorldSize(20.f);
	DemoText->SetTextRenderColor(FColor::White);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		DemoMesh->SetStaticMesh(CubeMesh.Object);
	}
}

void ANetLesson15TeamActor::BeginPlay()
{
	Super::BeginPlay();

	UpdateVisual();
}

bool ANetLesson15TeamActor::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	return GetViewerTeamId(RealViewer, ViewTarget) == TeamId;
}

void ANetLesson15TeamActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetLesson15TeamActor, TeamId);
	DOREPLIFETIME(ANetLesson15TeamActor, Value);
}

void ANetLesson15TeamActor::ToggleTeamOnServer()
{
	if (!HasAuthority())
	{
		return;
	}

	TeamId = TeamId == 1 ? 2 : 1;
	Value++;
	UpdateVisual();
}

void ANetLesson15TeamActor::OnRep_Visual()
{
	UpdateVisual();
}

void ANetLesson15TeamActor::UpdateVisual()
{
	const float Size = 0.8f + Value * 0.15f;
	DemoMesh->SetRelativeScale3D(FVector(Size, Size, Size));

	const FString MachineText = HasAuthority() ? TEXT("Server World") : TEXT("Team Client World");
	const FString Label = FString::Printf(
		TEXT("TeamOnly Trap\n%s\nCustom IsNetRelevantFor\nTeam=%d\nValue=%d"),
		*MachineText,
		TeamId,
		Value
	);
	DemoText->SetText(FText::FromString(Label));
}

int32 ANetLesson15TeamActor::GetViewerTeamId(const AActor* RealViewer, const AActor* ViewTarget) const
{
	const APawn* ViewerPawn = Cast<APawn>(ViewTarget);
	if (!ViewerPawn)
	{
		ViewerPawn = Cast<APawn>(RealViewer);
	}

	if (ViewerPawn)
	{
		if (const ANetLesson15PlayerState* LessonPlayerState = ViewerPawn->GetPlayerState<ANetLesson15PlayerState>())
		{
			return LessonPlayerState->GetTeamId();
		}
	}

	const AController* ViewerController = Cast<AController>(RealViewer);
	if (ViewerController)
	{
		if (const ANetLesson15PlayerState* LessonPlayerState = ViewerController->GetPlayerState<ANetLesson15PlayerState>())
		{
			return LessonPlayerState->GetTeamId();
		}
	}

	return 0;
}
