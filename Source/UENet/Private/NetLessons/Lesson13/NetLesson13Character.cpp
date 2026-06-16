// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson13/NetLesson13Character.h"

#include "Components/TextRenderComponent.h"
#include "EngineUtils.h"
#include "InputCoreTypes.h"
#include "NetLessons/Lesson13/NetLesson13AlwaysActor.h"

ANetLesson13Character::ANetLesson13Character()
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

void ANetLesson13Character::BeginPlay()
{
	Super::BeginPlay();

	UpdateLessonText();
}

void ANetLesson13Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindKey(EKeys::F, IE_Pressed, this, &ThisClass::RequestUpdateAlwaysActor);
}

void ANetLesson13Character::RequestUpdateAlwaysActor()
{
	if (HasAuthority())
	{
		ServerUpdateAlwaysActor_Implementation();
		return;
	}

	ServerUpdateAlwaysActor();
}

void ANetLesson13Character::ServerUpdateAlwaysActor_Implementation()
{
	for (TActorIterator<ANetLesson13AlwaysActor> It(GetWorld()); It; ++It)
	{
		It->AddValueOnServer();
	}
}

void ANetLesson13Character::UpdateLessonText()
{
	const FString MachineText = HasAuthority() ? TEXT("Server") : TEXT("Client");
	const FString Text = FString::Printf(
		TEXT("%s\nLesson13 AlwaysRelevant\nF: Server Value +1"),
		*MachineText
	);
	LessonText->SetText(FText::FromString(Text));
}
