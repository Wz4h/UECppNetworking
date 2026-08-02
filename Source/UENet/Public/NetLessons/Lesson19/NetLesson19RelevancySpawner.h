// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetLesson19RelevancySpawner.generated.h"

class ANetLesson19RelevancyCube;

/** Relevancy 演示模式：AlwaysRelevant 会同步给所有连接，CullDistance 会按距离裁剪。 */
UENUM(BlueprintType)
enum class ENetLesson19RelevancyMode : uint8
{
	AlwaysRelevant UMETA(DisplayName = "Always Relevant"),
	CullDistance UMETA(DisplayName = "Cull Distance")
};

/**
 * Relevancy 课程用的批量生成器。
 * BeginPlay 时只在服务器生成大量 replicated Cube，用 stat net 对比相关性裁剪前后的网络压力。
 */
UCLASS()
class UENET_API ANetLesson19RelevancySpawner : public AActor
{
	GENERATED_BODY()

public:
	/** 构造函数：关闭自身 Tick，默认只作为服务器生成器使用。 */
	ANetLesson19RelevancySpawner();

protected:
	/** 游戏开始时在服务器批量生成 Cube。 */
	virtual void BeginPlay() override;

	/** 要生成的 Cube 类，可以在 Details 中替换为 C++ 类或蓝图子类。 */
	UPROPERTY(EditAnywhere, Category = "Network Lesson 19")
	TSubclassOf<ANetLesson19RelevancyCube> CubeClass;

	/** 生成数量，用于制造明显的网络压力。 */
	UPROPERTY(EditAnywhere, Category = "Network Lesson 19", meta = (ClampMin = "1"))
	int32 SpawnCount = 1000;

	/** 网格间距，避免 Cube 完全重叠。 */
	UPROPERTY(EditAnywhere, Category = "Network Lesson 19", meta = (ClampMin = "1.0"))
	float GridSpacing = 300.f;

	/** 相关性模式：AlwaysRelevant 或按 CullDistance 裁剪。 */
	UPROPERTY(EditAnywhere, Category = "Network Lesson 19")
	ENetLesson19RelevancyMode RelevancyMode = ENetLesson19RelevancyMode::CullDistance;

	/** CullDistance 模式下的可见同步距离。 */
	UPROPERTY(EditAnywhere, Category = "Network Lesson 19", meta = (ClampMin = "1.0"))
	float CullDistance = 3000.f;

	/** 生成出的 Cube 使用的网络更新频率。 */
	UPROPERTY(EditAnywhere, Category = "Network Lesson 19", meta = (ClampMin = "0.1"))
	float CubeNetUpdateFrequency = 10.f;

private:
	/** 根据当前 Details 配置，对单个 Cube 应用网络相关性参数。 */
	void ConfigureCube(ANetLesson19RelevancyCube* Cube) const;
};
