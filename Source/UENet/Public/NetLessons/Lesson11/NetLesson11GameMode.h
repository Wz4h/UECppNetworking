// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NetLesson11GameMode.generated.h"

UCLASS()
class UENET_API ANetLesson11GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANetLesson11GameMode();

	virtual void BeginPlay() override;

protected:
	// 服务器生成三个演示 Actor，用来讲 ActorChannel 按 Actor 和 Connection 组织。
	void SpawnFrameworkActors();
};
