// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson14/NetLesson14Character.h"

#include "Components/TextRenderComponent.h"
#include "EngineUtils.h"
#include "InputCoreTypes.h"
#include "NetLessons/Lesson14/NetLesson14OwnerActor.h"

ANetLesson14Character::ANetLesson14Character()
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

void ANetLesson14Character::BeginPlay()
{
	Super::BeginPlay();

	UpdateLessonText();
}

void ANetLesson14Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindKey(EKeys::F, IE_Pressed, this, &ThisClass::RequestUpdateOwnerActor);
}

void ANetLesson14Character::RequestUpdateOwnerActor()
{
	if (HasAuthority())
	{
		ServerUpdateOwnerActor_Implementation();
		return;
	}

	ServerUpdateOwnerActor();
}

void ANetLesson14Character::ServerUpdateOwnerActor_Implementation()
{
	AController* MyController = GetController();
	for (TActorIterator<ANetLesson14OwnerActor> It(GetWorld()); It; ++It)
	{
		if (It->GetOwner() == MyController)
		{
			It->AddValueOnServer();
		}
	}
}

void ANetLesson14Character::UpdateLessonText()
{
	const FString MachineText = HasAuthority() ? TEXT("Server") : TEXT("Client");
	const FString Text = FString::Printf(
		TEXT("%s\nLesson14 OnlyRelevantToOwner\nF: Owner Value +1"),
		*MachineText
	);
	LessonText->SetText(FText::FromString(Text));
}
