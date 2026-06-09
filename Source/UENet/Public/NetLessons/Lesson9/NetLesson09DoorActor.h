// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetLesson09DoorActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class UENET_API ANetLesson09DoorActor : public AActor
{
	GENERATED_BODY()

public:
	ANetLesson09DoorActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 普通服务器函数，不是 RPC。由服务器收到 Character RPC 后调用。
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
