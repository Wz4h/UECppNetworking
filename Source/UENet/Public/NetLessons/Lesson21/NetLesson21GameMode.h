// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NetLessons/Lesson21/NetLesson21FrequencyMoverSpawner.h"
#include "NetLesson21GameMode.generated.h"

/**
 * Lesson21 的基础 GameMode。
 * 默认生成 Both 模式，用于肉眼对比高频同步和低频同步。
 */
UCLASS()
class UENET_API ANetLesson21GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	/** 构造函数：设置本课程默认 Pawn。 */
	ANetLesson21GameMode();

	/** 游戏开始时在服务器生成移动 Actor 对比组。 */
	virtual void BeginPlay() override;

protected:
	/** 当前 GameMode 要生成的模式，子类通过构造函数改成 HighOnly 或 LowOnly。 */
	UPROPERTY(EditDefaultsOnly, Category = "Network Lesson 21")
	ENetLesson21SpawnMode AutoSpawnMode = ENetLesson21SpawnMode::Both;

	/** 服务器生成 Spawner，并把 AutoSpawnMode 写入 Spawner。 */
	void SpawnFrequencyDemo();
};

/**
 * Lesson21 高频专用 GameMode。
 * 只生成 High 100 Hz 组，用于单独观察高频模式的 stat net。
 */
UCLASS()
class UENET_API ANetLesson21HighOnlyGameMode : public ANetLesson21GameMode
{
	GENERATED_BODY()

public:
	/** 构造函数：把生成模式改为 HighOnly。 */
	ANetLesson21HighOnlyGameMode();
};

/**
 * Lesson21 低频专用 GameMode。
 * 只生成 Low 0.5 Hz 组，用于单独观察低频模式的 stat net。
 */
UCLASS()
class UENET_API ANetLesson21LowOnlyGameMode : public ANetLesson21GameMode
{
	GENERATED_BODY()

public:
	/** 构造函数：把生成模式改为 LowOnly。 */
	ANetLesson21LowOnlyGameMode();
};
