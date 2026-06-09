// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson10/NetLesson10Character.h"

#include "EngineUtils.h"
#include "InputCoreTypes.h"
#include "Net/UnrealNetwork.h"
#include "NetLessons/Lesson10/NetLesson10DoorActor.h"
#include "NetLessons/Lesson10/NetLesson10InteractionComponent.h"
#include "NetLessons/Lesson10/NetLesson10WeaponActor.h"

ANetLesson10Character::ANetLesson10Character()
{
	bReplicates = true;

	InteractionComponent = CreateDefaultSubobject<UNetLesson10InteractionComponent>(TEXT("InteractionComponent"));
}

void ANetLesson10Character::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		static int32 WeaponSpawnIndex = 0;
		const FVector SpawnLocation = FVector(350.f, 120.f + WeaponSpawnIndex * 80.f, 160.f);
		++WeaponSpawnIndex;

		WeaponActor = GetWorld()->SpawnActor<ANetLesson10WeaponActor>(
			ANetLesson10WeaponActor::StaticClass(),
			SpawnLocation,
			FRotator::ZeroRotator
		);

		if (WeaponActor)
		{
			// WeaponActor 进入玩家拥有链后，客户端可以直接在 WeaponActor 上调用 Server RPC。
			WeaponActor->SetOwner(this);
		}
	}
}

void ANetLesson10Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindKey(EKeys::One, IE_Pressed, this, &ThisClass::TestInteractionComponentRPC);
	PlayerInputComponent->BindKey(EKeys::Three, IE_Pressed, this, &ThisClass::TestWeaponActorRPC);
}

void ANetLesson10Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetLesson10Character, WeaponActor);
}

void ANetLesson10Character::TestInteractionComponentRPC()
{
	if (!InteractionComponent)
	{
		return;
	}

	for (TActorIterator<ANetLesson10DoorActor> It(GetWorld()); It; ++It)
	{
		InteractionComponent->InteractWithDoor(*It);
		return;
	}
}

void ANetLesson10Character::TestWeaponActorRPC()
{
	if (WeaponActor)
	{
		WeaponActor->Fire();
	}
}
