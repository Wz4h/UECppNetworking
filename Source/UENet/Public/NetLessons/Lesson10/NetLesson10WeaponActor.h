// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetLesson10WeaponActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class UENET_API ANetLesson10WeaponActor : public AActor
{
	GENERATED_BODY()

public:
	ANetLesson10WeaponActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void Fire();

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* WeaponMesh;

	UPROPERTY(ReplicatedUsing = OnRep_ScaleValue)
	float ScaleValue = 1.f;

	// Server RPC：WeaponActor 通过 SetOwner 进入玩家拥有链，因此客户端可以直接调用。
	UFUNCTION(Server, Reliable)
	void Server_Fire();

	UFUNCTION()
	void OnRep_ScaleValue();

	void UpdateWeaponScale();

	void ShowScreenMessage(const FString& Message, const FColor& Color) const;
};
