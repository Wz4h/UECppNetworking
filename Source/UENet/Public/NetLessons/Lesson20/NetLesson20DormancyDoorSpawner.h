// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetLesson20DormancyDoorSpawner.generated.h"

class ANetLesson20DormancyDoor;

/**
 * Dormancy 课程用的门生成器。
 * BeginPlay 时只在服务器生成一批门，用于观察开启 Dormancy 后静止 Actor 的网络压力变化。
 */
UCLASS()
class UENET_API ANetLesson20DormancyDoorSpawner : public AActor
{
	GENERATED_BODY()

public:
	/** 构造函数：默认使用 C++ DormancyDoor 类。 */
	ANetLesson20DormancyDoorSpawner();

protected:
	/** 游戏开始时在服务器生成门。 */
	virtual void BeginPlay() override;

	/** 要生成的门类，可以替换为 C++ 类或蓝图子类。 */
	UPROPERTY(EditAnywhere, Category = "Network Lesson 20")
	TSubclassOf<ANetLesson20DormancyDoor> DoorClass;

	/** 生成门的数量，默认 100。 */
	UPROPERTY(EditAnywhere, Category = "Network Lesson 20", meta = (ClampMin = "1"))
	int32 SpawnCount = 100;

	/** 门之间的间距。 */
	UPROPERTY(EditAnywhere, Category = "Network Lesson 20", meta = (ClampMin = "1.0"))
	float GridSpacing = 300.f;

	/** 是否让生成出的门使用 Dormancy。 */
	UPROPERTY(EditAnywhere, Category = "Network Lesson 20")
	bool bUseDormancy = true;
};
