// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NetLesson10InteractionComponent.generated.h"

class ANetLesson10DoorActor;

UCLASS(ClassGroup=(NetworkLessons), meta=(BlueprintSpawnableComponent))
class UENET_API UNetLesson10InteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNetLesson10InteractionComponent();

	void InteractWithDoor(ANetLesson10DoorActor* TargetDoor);

protected:
	// Server RPC：交互系统自己的服务器请求入口。
	UFUNCTION(Server, Reliable)
	void Server_InteractWithDoor(ANetLesson10DoorActor* TargetDoor);

	void ShowScreenMessage(const FString& Message, const FColor& Color) const;
};
