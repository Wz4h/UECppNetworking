// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson15/NetLesson15Character.h"

#include "Components/TextRenderComponent.h"
#include "EngineUtils.h"
#include "InputCoreTypes.h"
#include "NetLessons/Lesson15/NetLesson15PlayerState.h"
#include "NetLessons/Lesson15/NetLesson15TeamActor.h"

ANetLesson15Character::ANetLesson15Character()
{
	bReplicates = true;

	LessonText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("LessonText"));
	LessonText->SetupAttachment(GetRootComponent());
	LessonText->SetRelativeLocation(FVector(0.f, 0.f, 130.f));
	LessonText->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	LessonText->SetHorizontalAlignment(EHTA_Center);
	LessonText->SetWorldSize(22.f);
	LessonText->SetTextRenderColor(FColor::Cyan);
}

void ANetLesson15Character::BeginPlay()
{
	Super::BeginPlay();

	UpdateLessonText();
}

void ANetLesson15Character::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	UpdateLessonText();
}

void ANetLesson15Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindKey(EKeys::F, IE_Pressed, this, &ThisClass::RequestToggleTeamActor);
}

void ANetLesson15Character::UpdateLessonText()
{
	int32 TeamId = 0;
	if (const ANetLesson15PlayerState* LessonPlayerState = GetPlayerState<ANetLesson15PlayerState>())
	{
		TeamId = LessonPlayerState->GetTeamId();
	}

	const FString MachineText = HasAuthority() ? TEXT("Server") : TEXT("Client");
	const FString Text = FString::Printf(
		TEXT("%s\nLesson15 Team Relevancy\nTeam=%d\nF: Trap Team 1 / 2"),
		*MachineText,
		TeamId
	);
	LessonText->SetText(FText::FromString(Text));
}

void ANetLesson15Character::RequestToggleTeamActor()
{
	if (HasAuthority())
	{
		ServerToggleTeamActor_Implementation();
		return;
	}

	ServerToggleTeamActor();
}

void ANetLesson15Character::ServerToggleTeamActor_Implementation()
{
	for (TActorIterator<ANetLesson15TeamActor> It(GetWorld()); It; ++It)
	{
		It->ToggleTeamOnServer();
	}
}
