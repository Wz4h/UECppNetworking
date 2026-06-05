// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson7/NetLesson07Visualizer.h"

#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"

ANetLesson07Visualizer::ANetLesson07Visualizer()
{
	bReplicates = false;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	TitleText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TitleText"));
	TitleText->SetupAttachment(SceneRoot);
	TitleText->SetRelativeLocation(FVector(0.f, 0.f, 140.f));
	TitleText->SetWorldSize(34.f);
	TitleText->SetHorizontalAlignment(EHTA_Center);

	GameModeText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("GameModeText"));
	GameModeText->SetupAttachment(SceneRoot);
	GameModeText->SetRelativeLocation(FVector(0.f, 0.f, 80.f));

	GameStateText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("GameStateText"));
	GameStateText->SetupAttachment(SceneRoot);
	GameStateText->SetRelativeLocation(FVector(0.f, 0.f, 40.f));

	PlayerControllerText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("PlayerControllerText"));
	PlayerControllerText->SetupAttachment(SceneRoot);

	PlayerStateText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("PlayerStateText"));
	PlayerStateText->SetupAttachment(SceneRoot);
	PlayerStateText->SetRelativeLocation(FVector(0.f, 0.f, -40.f));

	PawnText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("PawnText"));
	PawnText->SetupAttachment(SceneRoot);
	PawnText->SetRelativeLocation(FVector(0.f, 0.f, -80.f));

	for (UTextRenderComponent* TextComponent : {GameModeText, GameStateText, PlayerControllerText, PlayerStateText, PawnText})
	{
		TextComponent->SetWorldSize(26.f);
		TextComponent->SetHorizontalAlignment(EHTA_Center);
	}
}

void ANetLesson07Visualizer::UpdateFrameworkView(bool bIsServerWorld, bool bHasGameMode, bool bHasGameState,
	int32 PlayerControllerCount, int32 PlayerStateCount, int32 PawnCount)
{
	TitleText->SetText(FText::FromString(bIsServerWorld ? TEXT("SERVER WORLD") : TEXT("CLIENT WORLD")));
	TitleText->SetTextRenderColor(bIsServerWorld ? FColor::Yellow : FColor::Cyan);

	SetPresenceText(GameModeText, TEXT("GameMode"), bHasGameMode);
	SetPresenceText(GameStateText, TEXT("GameState"), bHasGameState);
	SetCountText(PlayerControllerText, TEXT("PlayerControllers"), PlayerControllerCount);
	SetCountText(PlayerStateText, TEXT("PlayerStates"), PlayerStateCount);
	SetCountText(PawnText, TEXT("Pawns"), PawnCount);
}

void ANetLesson07Visualizer::SetPresenceText(UTextRenderComponent* TextComponent, const TCHAR* Label, bool bPresent)
{
	TextComponent->SetText(FText::FromString(FString::Printf(
		TEXT("%s  [%s]"),
		Label,
		bPresent ? TEXT("PRESENT") : TEXT("ABSENT")
	)));
	TextComponent->SetTextRenderColor(bPresent ? FColor::Green : FColor::Red);
}

void ANetLesson07Visualizer::SetCountText(UTextRenderComponent* TextComponent, const TCHAR* Label, int32 Count)
{
	const FString CountBar = FString::ChrN(FMath::Max(Count, 0), TEXT('|'));
	TextComponent->SetText(FText::FromString(FString::Printf(TEXT("%s  %d  %s"), Label, Count, *CountBar)));
	TextComponent->SetTextRenderColor(Count > 0 ? FColor::Green : FColor::Red);
}
