// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetLesson08DoorActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class UENET_API ANetLesson08DoorActor : public AActor
{
	GENERATED_BODY()

public:
	ANetLesson08DoorActor();

	// Demo1：客户端直接调用公共 Door 的 Server RPC。因为客户端不拥有 Door，所以服务器不会执行。
	UFUNCTION(Server, Reliable)
	void Server_TestDoorRPC();

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* DoorMesh;

	void ShowScreenMessage(const FString& Message, const FColor& Color) const;
};
