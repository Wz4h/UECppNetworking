// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson18/NetLesson18Character.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputCoreTypes.h"
#include "UObject/ConstructorHelpers.h"

ANetLesson18Character::ANetLesson18Character()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MaxFlySpeed = FlySpeed;
	GetCharacterMovement()->BrakingDecelerationFlying = 2048.f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(GetCapsuleComponent());
	Camera->SetRelativeLocation(FVector(0.f, 0.f, 70.f));
	Camera->bUsePawnControlRotation = true;

	DemoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DemoMesh"));
	DemoMesh->SetupAttachment(GetCapsuleComponent());
	DemoMesh->SetRelativeLocation(FVector(0.f, 0.f, -20.f));
	DemoMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 1.8f));
	DemoMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DemoMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	DemoMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	DebugText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("DebugText"));
	DebugText->SetupAttachment(GetCapsuleComponent());
	DebugText->SetRelativeLocation(FVector(0.f, 0.f, 130.f));
	DebugText->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	DebugText->SetHorizontalAlignment(EHTA_Center);
	DebugText->SetWorldSize(18.f);
	DebugText->SetTextRenderColor(FColor::White);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		DemoMesh->SetStaticMesh(CubeMesh.Object);
	}
}

void ANetLesson18Character::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	UpdateDebugText();
}

void ANetLesson18Character::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateDebugText();
}

void ANetLesson18Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ThisClass::Input_Fire);
	PlayerInputComponent->BindAxisKey(EKeys::W, this, &ThisClass::Input_MoveForward);
	PlayerInputComponent->BindAxisKey(EKeys::S, this, &ThisClass::Input_MoveBackward);
	PlayerInputComponent->BindAxisKey(EKeys::D, this, &ThisClass::Input_MoveRight);
	PlayerInputComponent->BindAxisKey(EKeys::A, this, &ThisClass::Input_MoveLeft);
	PlayerInputComponent->BindAxisKey(EKeys::SpaceBar, this, &ThisClass::Input_MoveUp);
	PlayerInputComponent->BindAxisKey(EKeys::LeftControl, this, &ThisClass::Input_MoveDown);
	PlayerInputComponent->BindAxisKey(EKeys::MouseX, this, &ThisClass::Input_Turn);
	PlayerInputComponent->BindAxisKey(EKeys::MouseY, this, &ThisClass::Input_LookUp);
}

void ANetLesson18Character::Input_Fire()
{
	FVector TraceStart = Camera->GetComponentLocation();
	FRotator TraceRotation = Camera->GetComponentRotation();

	if (AController* OwnerController = GetController())
	{
		TraceRotation = OwnerController->GetControlRotation();
	}

	ShowFireMessage(TEXT("Client Request Fire"), FColor::Yellow);
	ServerFire(TraceStart, TraceRotation);
}

void ANetLesson18Character::Input_MoveForward(float Value)
{
	if (FMath::IsNearlyZero(Value))
	{
		return;
	}

	const FRotator ControlRotation = Controller ? Controller->GetControlRotation() : GetActorRotation();
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Value);
}

void ANetLesson18Character::Input_MoveBackward(float Value)
{
	if (FMath::IsNearlyZero(Value))
	{
		return;
	}

	const FRotator ControlRotation = Controller ? Controller->GetControlRotation() : GetActorRotation();
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), -Value);
}

void ANetLesson18Character::Input_MoveRight(float Value)
{
	if (FMath::IsNearlyZero(Value))
	{
		return;
	}

	const FRotator ControlRotation = Controller ? Controller->GetControlRotation() : GetActorRotation();
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Value);
}

void ANetLesson18Character::Input_MoveLeft(float Value)
{
	if (FMath::IsNearlyZero(Value))
	{
		return;
	}

	const FRotator ControlRotation = Controller ? Controller->GetControlRotation() : GetActorRotation();
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), -Value);
}

void ANetLesson18Character::Input_MoveUp(float Value)
{
	if (FMath::IsNearlyZero(Value))
	{
		return;
	}

	AddMovementInput(FVector::UpVector, Value);
}

void ANetLesson18Character::Input_MoveDown(float Value)
{
	if (FMath::IsNearlyZero(Value))
	{
		return;
	}

	AddMovementInput(FVector::UpVector, -Value);
}

void ANetLesson18Character::Input_Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ANetLesson18Character::Input_LookUp(float Value)
{
	AddControllerPitchInput(-Value);
}

void ANetLesson18Character::ServerFire_Implementation(FVector_NetQuantize TraceStart, FRotator TraceRotation)
{
	PerformServerTrace(TraceStart, TraceRotation);
}

void ANetLesson18Character::PerformServerTrace(const FVector& TraceStart, const FRotator& TraceRotation)
{
	if (!HasAuthority() || !GetWorld())
	{
		return;
	}

	const FVector TraceEnd = TraceStart + TraceRotation.Vector() * FireRange;

	FHitResult Hit;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(Lesson18ServerFireTrace), true, this);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

	const FVector DebugEnd = bHit ? Hit.ImpactPoint : TraceEnd;
	const FColor TraceColor = bHit ? FColor::Green : FColor::Red;

	if (bDrawDebugTrace)
	{
		DrawDebugLine(GetWorld(), TraceStart, DebugEnd, TraceColor, false, 3.f, 0, 2.f);

		if (bHit)
		{
			DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 16.f, 12, FColor::Green, false, 3.f);
		}
	}

	if (bHit && Hit.GetActor())
	{
		const FString Message = FString::Printf(TEXT("Server Hit: %s Damage=%.1f"), *Hit.GetActor()->GetName(), HitDamage);
		ShowFireMessage(Message, FColor::Green);
	}
	else
	{
		ShowFireMessage(TEXT("Server Miss"), FColor::Red);
	}
}

void ANetLesson18Character::UpdateDebugText()
{
	const FString MachineText = HasAuthority() ? TEXT("Server") : TEXT("Client");
	const FString Text = FString::Printf(
		TEXT("Lesson18 Lag Compensation Step 1\nMachine=%s LocalRole=%s RemoteRole=%s\nNo Lag Compensation\nLeft Mouse: Server Current-World Trace\nFireRange=%.0f"),
		*MachineText,
		*NetRoleToString(GetLocalRole()),
		*NetRoleToString(GetRemoteRole()),
		FireRange
	);

	DebugText->SetText(FText::FromString(Text));
}

void ANetLesson18Character::ShowFireMessage(const FString& Message, const FColor& Color) const
{
	const FString FullMessage = FString::Printf(TEXT("[Lesson18] %s"), *Message);
	UE_LOG(LogTemp, Warning, TEXT("%s"), *FullMessage);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.5f, Color, FullMessage);
	}
}

FString ANetLesson18Character::NetRoleToString(ENetRole InRole) const
{
	switch (InRole)
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
