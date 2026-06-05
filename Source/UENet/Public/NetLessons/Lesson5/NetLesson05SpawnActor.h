// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetLesson05SpawnActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class UENET_API ANetLesson05SpawnActor : public AActor
{
	GENERATED_BODY()

public:
	ANetLesson05SpawnActor();

protected:
	// 服务器创建后复制给客户端观察的简单盒子。
	UPROPERTY(VisibleAnywhere, Category = "Network Lesson 05")
	UStaticMeshComponent* MeshComponent;
};
