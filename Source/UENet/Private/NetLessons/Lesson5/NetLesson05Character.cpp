// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson5/NetLesson05Character.h"

#include "Engine/Engine.h"
#include "InputCoreTypes.h"
#include "NetLessons/Lesson5/NetLesson05SpawnActor.h"

ANetLesson05Character::ANetLesson05Character()
{
	bReplicates = true;
}

void ANetLesson05Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindKey(
		EKeys::F,
		IE_Pressed,
		this,
		&ThisClass::TestSpawnActor
	);
}

void ANetLesson05Character::TestSpawnActor()
{
	PrintNetworkDebugInfo(TEXT("InputPressed"));

	if (HasAuthority())
	{
		SpawnActorOnServer();
		return;
	}

	SpawnActorOnServer();
}

void ANetLesson05Character::ServerSpawnActor_Implementation()
{
	PrintNetworkDebugInfo(TEXT("ServerSpawnActor"));

	SpawnActorOnServer();
}

void ANetLesson05Character::SpawnActorOnServer()
{


	const int32 SpawnIndex = SpawnedActorCount++;
	const FVector ForwardOffset = GetActorForwardVector() * 220.f;
	const FVector RightOffset = GetActorRightVector() * static_cast<float>((SpawnIndex % 5) * 120 - 240);
	const FVector RowOffset = GetActorForwardVector() * static_cast<float>((SpawnIndex / 5) * 120);
	const FVector SpawnLocation = GetActorLocation() + ForwardOffset + RightOffset + RowOffset + FVector(0.f, 0.f, 80.f);
	const FRotator SpawnRotation = FRotator::ZeroRotator;

	GetWorld()->SpawnActor<ANetLesson05SpawnActor>(
		ANetLesson05SpawnActor::StaticClass(),
		SpawnLocation,
		SpawnRotation
	);

	PrintNetworkDebugInfo(TEXT("SpawnActorOnServer"));
}

void ANetLesson05Character::PrintNetworkDebugInfo(const FString& Prefix) const
{
	const FString MachineText = HasAuthority() ? TEXT("Server") : TEXT("Client");

	const FString Message = FString::Printf(
		TEXT("[Lesson5][%s] Machine=%s Actor=%s LocalRole=%s RemoteRole=%s"),
		*Prefix,
		*MachineText,
		*GetName(),
		*NetRoleToString(GetLocalRole()),
		*NetRoleToString(GetRemoteRole())
	);

	UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			3.f,
			FColor::Yellow,
			Message
		);
	}
}

FString ANetLesson05Character::NetRoleToString(ENetRole Role)
{
	switch (Role)
	{
	case ROLE_None:
		return TEXT("None");

	case ROLE_SimulatedProxy:
		return TEXT("SimulatedProxy");

	case ROLE_AutonomousProxy:
		return TEXT("AutonomousProxy");

	case ROLE_Authority:
		return TEXT("Authority");

	default:
		return TEXT("Unknown");
	}
}
