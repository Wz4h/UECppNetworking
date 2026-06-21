// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson12/NetLesson12Character.h"

#include "Components/TextRenderComponent.h"
#include "EngineUtils.h"
#include "InputCoreTypes.h"
#include "NetLessons/Lesson12/NetLesson12RelevancyActor.h"

ANetLesson12Character::ANetLesson12Character()
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

void ANetLesson12Character::BeginPlay()
{
	Super::BeginPlay();

	UpdateLessonText();
}

void ANetLesson12Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindKey(EKeys::F, IE_Pressed, this, &ThisClass::RequestToggleDistanceActor);
}

void ANetLesson12Character::UpdateLessonText()
{
	const FString MachineText = HasAuthority() ? TEXT("Server") : TEXT("Client");
	const FString Text = FString::Printf(
		TEXT("%s\nLesson12 Distance Relevancy\nF: Near / Far"),
		*MachineText
	);

	LessonText->SetText(FText::FromString(Text));
}

void ANetLesson12Character::RequestToggleDistanceActor()
{
	if (HasAuthority())
	{
		ServerToggleDistanceActor_Implementation();
		return;
	}

	ServerToggleDistanceActor();
}

void ANetLesson12Character::ServerToggleDistanceActor_Implementation()
{
	for (TActorIterator<ANetLesson12RelevancyActor> It(GetWorld()); It; ++It)
	{
		if (It->ActorHasTag(TEXT("Lesson12_Distance")))
		{
			const bool bIsFar = It->GetActorLocation().X > 1500.f;
			It->SetActorLocation(bIsFar ? FVector(520.f, 0.f, 80.f) : FVector(4000.f, 0.f, 80.f));
			It->AddValueOnServer();
			It->ForceNetUpdate();
		}
	}
}
