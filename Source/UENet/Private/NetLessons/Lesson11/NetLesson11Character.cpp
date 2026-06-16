// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson11/NetLesson11Character.h"

#include "Components/TextRenderComponent.h"
#include "EngineUtils.h"
#include "InputCoreTypes.h"
#include "NetLessons/Lesson11/NetLesson11FrameworkActor.h"

ANetLesson11Character::ANetLesson11Character()
{
	bReplicates = true;

	FrameworkText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("FrameworkText"));
	FrameworkText->SetupAttachment(GetRootComponent());
	FrameworkText->SetRelativeLocation(FVector(0.f, 0.f, 130.f));
	FrameworkText->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	FrameworkText->SetHorizontalAlignment(EHTA_Center);
	FrameworkText->SetWorldSize(20.f);
	FrameworkText->SetTextRenderColor(FColor::Cyan);
}

void ANetLesson11Character::BeginPlay()
{
	Super::BeginPlay();

	UpdateFrameworkText();
}

void ANetLesson11Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindKey(EKeys::F, IE_Pressed, this, &ThisClass::RequestChangeBox);
	PlayerInputComponent->BindKey(EKeys::G, IE_Pressed, this, &ThisClass::RequestToggleCoinRelevancy);
	PlayerInputComponent->BindKey(EKeys::H, IE_Pressed, this, &ThisClass::RequestDestroyDoor);
}

void ANetLesson11Character::RequestChangeBox()
{
	if (HasAuthority())
	{
		ServerChangeBox_Implementation();
		return;
	}

	ServerChangeBox();
}

void ANetLesson11Character::RequestToggleCoinRelevancy()
{
	if (HasAuthority())
	{
		ServerToggleCoinRelevancy_Implementation();
		return;
	}

	ServerToggleCoinRelevancy();
}

void ANetLesson11Character::RequestDestroyDoor()
{
	if (HasAuthority())
	{
		ServerDestroyDoor_Implementation();
		return;
	}

	ServerDestroyDoor();
}

void ANetLesson11Character::ServerChangeBox_Implementation()
{
	for (TActorIterator<ANetLesson11FrameworkActor> It(GetWorld()); It; ++It)
	{
		if (It->ActorHasTag(TEXT("Lesson11_Box")))
		{
			It->AddDemoValueOnServer();
		}
	}
}

void ANetLesson11Character::ServerToggleCoinRelevancy_Implementation()
{
	for (TActorIterator<ANetLesson11FrameworkActor> It(GetWorld()); It; ++It)
	{
		if (It->ActorHasTag(TEXT("Lesson11_Coin")))
		{
			const bool bCoinIsFar = It->GetActorLocation().X > 1500.f;
			const FVector NewLocation = bCoinIsFar ? FVector(350.f, 260.f, 80.f) : FVector(3000.f, 260.f, 80.f);
			It->SetServerLocation(NewLocation);
		}
	}
}

void ANetLesson11Character::ServerDestroyDoor_Implementation()
{
	for (TActorIterator<ANetLesson11FrameworkActor> It(GetWorld()); It; ++It)
	{
		if (It->ActorHasTag(TEXT("Lesson11_Door")))
		{
			It->Destroy();
			return;
		}
	}
}

void ANetLesson11Character::UpdateFrameworkText()
{
	const FString MachineText = HasAuthority() ? TEXT("Server") : TEXT("Client");
	const FString Text = FString::Printf(
		TEXT("%s\nCharacter owns Server RPC entry\nF: Replicate Box\nG: Relevancy Coin\nH: Destroy Door"),
		*MachineText
	);

	FrameworkText->SetText(FText::FromString(Text));
}
