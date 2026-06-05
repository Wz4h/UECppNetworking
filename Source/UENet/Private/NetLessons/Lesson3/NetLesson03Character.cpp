// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson3/NetLesson03Character.h"

#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputCoreTypes.h"
#include "Net/UnrealNetwork.h"

ANetLesson03Character::ANetLesson03Character()
{
	bReplicates = true;

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

void ANetLesson03Character::BeginPlay()
{
	Super::BeginPlay();

	InitialMeshScale = GetMesh()->GetRelativeScale3D();
	UpdateMeshScaleByValue();
}

void ANetLesson03Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindKey(
		EKeys::F,
		IE_Pressed,
		this,
		&ThisClass::TestServerDamage
	);
}

void ANetLesson03Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetLesson03Character, Health);
}

void ANetLesson03Character::TestServerDamage()
{
		Health -= 10.f;

		UpdateMeshScaleByValue();

		PrintNetworkDebugInfo(TEXT("ClientLocalDamage"));

	// 服务器窗口当前控制的不是客户端角色，所以这里刷新服务器世界里的所有角色权威副本。
	for (TActorIterator<ANetLesson03Character> It(GetWorld()); It; ++It)
	{
		ANetLesson03Character* LessonCharacter = *It;

		LessonCharacter->Health += 1.f;
		LessonCharacter->UpdateMeshScaleByValue();
		LessonCharacter->PrintNetworkDebugInfo(TEXT("ServerRefreshHealth"));
	}
}

void ANetLesson03Character::OnRep_Health()
{
	UpdateMeshScaleByValue();

	PrintNetworkDebugInfo(TEXT("OnRep_Health"));
}

void ANetLesson03Character::UpdateMeshScaleByValue()
{
	const float HealthRatio = FMath::Clamp(Health / 100.f, 0.5f, 1.f);

	GetMesh()->SetRelativeScale3D(InitialMeshScale * HealthRatio);
}

void ANetLesson03Character::PrintNetworkDebugInfo(const FString& Prefix) const
{
	const FString MachineText = HasAuthority() ? TEXT("Server") : TEXT("Client");
	const FVector MeshScale = GetMesh()->GetRelativeScale3D();

	const FString Message = FString::Printf(
		TEXT("[Lesson3][%s] Machine=%s Actor=%s LocalRole=%s RemoteRole=%s Health=%.1f MeshScale=%.2f"),
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

FString ANetLesson03Character::NetRoleToString(ENetRole Role)
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
