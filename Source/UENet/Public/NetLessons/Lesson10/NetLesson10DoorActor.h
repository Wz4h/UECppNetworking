// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetLesson10DoorActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class UENET_API ANetLesson10DoorActor : public AActor
{
	GENERATED_BODY()

public:
	ANetLesson10DoorActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 普通服务器函数，由 InteractionComponent 的 Server RPC 调用。
	void OpenDoor();

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* DoorMesh;

	UPROPERTY(ReplicatedUsing = OnRep_IsOpen)
	bool bIsOpen = false;

	UFUNCTION()
	void OnRep_IsOpen();

	void UpdateDoorVisual();
};
