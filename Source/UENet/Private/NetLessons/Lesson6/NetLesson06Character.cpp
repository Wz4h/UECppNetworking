// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson6/NetLesson06Character.h"

#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "InputCoreTypes.h"
#include "NetLessons/Lesson6/NetLesson06ScaleActor.h"

ANetLesson06Character::ANetLesson06Character()
{
	bReplicates = true;
}

void ANetLesson06Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindKey(
		EKeys::F,
		IE_Pressed,
		this,
		&ThisClass::TestChangeScaleValue
	);
}

void ANetLesson06Character::TestChangeScaleValue()
{
	PrintNetworkDebugInfo(TEXT("InputPressed"));

	if (HasAuthority())
	{
		ChangeScaleValueOnServer();
		return;
	}

	ServerChangeScaleValue();
}

void ANetLesson06Character::ServerChangeScaleValue_Implementation()
{
	PrintNetworkDebugInfo(TEXT("ServerChangeScaleValue"));

	ChangeScaleValueOnServer();
}

void ANetLesson06Character::ChangeScaleValueOnServer()
{
	if (!HasAuthority())
	{
		return;
	}

	for (TActorIterator<ANetLesson06ScaleActor> It(GetWorld()); It; ++It)
	{
		It->AddScaleValueOnServer();
	}
}

void ANetLesson06Character::PrintNetworkDebugInfo(const FString& Prefix) const
{
	const FString MachineText = HasAuthority() ? TEXT("Server") : TEXT("Client");

	const FString Message = FString::Printf(
		TEXT("[Lesson6][%s] Machine=%s Actor=%s LocalRole=%s RemoteRole=%s"),
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

FString ANetLesson06Character::NetRoleToString(ENetRole Role)
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
