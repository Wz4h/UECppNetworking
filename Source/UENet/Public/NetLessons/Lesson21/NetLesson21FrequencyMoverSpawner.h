// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetLesson21FrequencyMoverSpawner.generated.h"

class ANetLesson21FrequencyMover;

/** Lesson21 生成模式：同时生成两组用于视觉对比，或只生成单组用于 stat net 对比。 */
UENUM(BlueprintType)
enum class ENetLesson21SpawnMode : uint8
{
	Both UMETA(DisplayName = "Both"),
	HighOnly UMETA(DisplayName = "High Only"),
	LowOnly UMETA(DisplayName = "Low Only")
};

/**
 * Lesson21 的跑道生成器。
 * 用两条清晰跑道对比高频同步和低频同步；单组模式用于公平观察 stat net 差距。
 */
UCLASS()
class UENET_API ANetLesson21FrequencyMoverSpawner : public AActor
{
	GENERATED_BODY()

public:
	/** 构造函数：默认使用 Lesson21 的移动 Actor。 */
	ANetLesson21FrequencyMoverSpawner();

	/** 设置生成模式，GameMode 使用延迟生成时会先调用它再触发 BeginPlay。 */
	void SetSpawnMode(ENetLesson21SpawnMode NewSpawnMode);

protected:
	/** 游戏开始时只在服务器生成标题和移动 Actor。 */
	virtual void BeginPlay() override;

	/** 要生成的移动 Actor 类，可以替换为 C++ 类或蓝图子类。 */
	UPROPERTY(EditAnywhere, Category = "Network Lesson 21")
	TSubclassOf<ANetLesson21FrequencyMover> MoverClass;

	/** 生成模式：Both 用于视觉演示，HighOnly / LowOnly 用于 stat net 对比。 */
	UPROPERTY(EditAnywhere, Category = "Network Lesson 21")
	ENetLesson21SpawnMode SpawnMode = ENetLesson21SpawnMode::Both;

	/** 每组生成多少个 Actor，默认数量兼顾画面清晰和 stat net 差异。 */
	UPROPERTY(EditAnywhere, Category = "Network Lesson 21", meta = (ClampMin = "1"))
	int32 SpawnCountPerGroup = 24;

	/** 每条跑道内 Actor 沿 Y 轴的间距。 */
	UPROPERTY(EditAnywhere, Category = "Network Lesson 21", meta = (ClampMin = "1.0"))
	float ActorSpacing = 180.f;

	/** 高频跑道和低频跑道之间的 X 轴距离。 */
	UPROPERTY(EditAnywhere, Category = "Network Lesson 21", meta = (ClampMin = "1.0"))
	float LaneSpacing = 1800.f;

	/** 服务器移动距离。 */
	UPROPERTY(EditAnywhere, Category = "Network Lesson 21", meta = (ClampMin = "1.0"))
	float MoveDistance = 650.f;

	/** 服务器移动速度，速度越快低频组越容易看出跳变。 */
	UPROPERTY(EditAnywhere, Category = "Network Lesson 21", meta = (ClampMin = "1.0"))
	float MoveSpeed = 700.f;

private:
	/** 是否应该生成高频组。 */
	bool ShouldSpawnHighFrequencyGroup() const;

	/** 是否应该生成低频组。 */
	bool ShouldSpawnLowFrequencyGroup() const;

	/** 当前模式是否只生成一条跑道。 */
	bool IsSingleLaneMode() const;

	/** 根据模式和组别计算跑道 X 位置。 */
	float GetLaneX(bool bHighFrequencyGroup) const;

	/** 按指定模式生成一条直线跑道。 */
	void SpawnLane(bool bHighFrequencyGroup);

	/** 生成可复制的标题 Actor，说明当前跑道含义。 */
	void SpawnLaneLabel(const FString& LabelText, const FVector& Location, const FColor& Color);
};
