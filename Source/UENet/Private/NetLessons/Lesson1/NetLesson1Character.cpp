// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson1/NetLesson1Character.h"

#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputCoreTypes.h"

ANetLesson1Character::ANetLesson1Character()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.f;
}

void ANetLesson1Character::BeginPlay()
{
	Super::BeginPlay();

	InitialMeshScale = GetMesh()->GetRelativeScale3D();
}

void ANetLesson1Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindKey(
		EKeys::F,
		IE_Pressed,
		this,
		&ThisClass::TestLocalDamage
	);
}

void ANetLesson1Character::TestLocalDamage()
{
	Health -= 10.f;

	ApplyLocalMeshDamageEffect();

	PrintNetworkDebugInfo(TEXT("TestLocalDamage"));
}

void ANetLesson1Character::ApplyLocalMeshDamageEffect()
{
	const float HealthRatio = FMath::Clamp(Health / 100.f, 0.5f, 1.f);

	GetMesh()->SetRelativeScale3D(InitialMeshScale * HealthRatio);
}

void ANetLesson1Character::PrintNetworkDebugInfo(const FString& Prefix) const
{
	const FString MachineText = HasAuthority() ? TEXT("Server") : TEXT("Client");
	const FVector MeshScale = GetMesh()->GetRelativeScale3D();

	const FString Message = FString::Printf(
		TEXT("[Lesson1][%s] Machine=%s Actor=%s LocalRole=%s RemoteRole=%s Health=%.1f MeshScale=%.2f"),
		*Prefix,
		*MachineText,
		*GetName(),
		*NetRoleToString(GetLocalRole()),
		*NetRoleToString(GetRemoteRole()),
		Health,
		MeshScale.X
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

FString ANetLesson1Character::NetRoleToString(ENetRole Role)
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
