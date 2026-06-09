// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson10/NetLesson10WeaponActor.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ANetLesson10WeaponActor::ANetLesson10WeaponActor()
{
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		WeaponMesh->SetStaticMesh(SphereMesh.Object);
	}

	UpdateWeaponScale();
}

void ANetLesson10WeaponActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetLesson10WeaponActor, ScaleValue);
}

void ANetLesson10WeaponActor::Fire()
{
	ShowScreenMessage(TEXT("[Lesson10] WeaponActor requested Server_Fire"), FColor::Yellow);
	Server_Fire();
}

void ANetLesson10WeaponActor::Server_Fire_Implementation()
{
	ScaleValue += 0.25f;
	UpdateWeaponScale();

	ShowScreenMessage(TEXT("[Lesson10] WeaponActor Server RPC executed"), FColor::Green);
}

void ANetLesson10WeaponActor::OnRep_ScaleValue()
{
	UpdateWeaponScale();
}

void ANetLesson10WeaponActor::UpdateWeaponScale()
{
	WeaponMesh->SetRelativeScale3D(FVector(ScaleValue * 0.35f));
}

void ANetLesson10WeaponActor::ShowScreenMessage(const FString& Message, const FColor& Color) const
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, Color, Message);
	}
}
