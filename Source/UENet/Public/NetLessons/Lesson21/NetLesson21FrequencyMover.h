// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetLesson21FrequencyMover.generated.h"

class UStaticMeshComponent;

/** 网络更新频率演示模式：高频更平滑但更耗网络，低频更省但移动更跳。 */
UENUM(BlueprintType)
enum class ENetLesson21FrequencyMode : uint8
{
	HighFrequency UMETA(DisplayName = "High Frequency"),
	LowFrequency UMETA(DisplayName = "Low Frequency")
};

/**
 * Lesson21 的移动 Actor。
 * 只在服务器 Tick 中移动，通过 ReplicatedMovement 按 NetUpdateFrequency 同步给客户端。
 */
UCLASS()
class UENET_API ANetLesson21FrequencyMover : public AActor
{
	GENERATED_BODY()

public:
	/** 构造函数：创建 Mesh，开启 Actor 复制和 ReplicatedMovement。 */
	ANetLesson21FrequencyMover();

	/** 游戏开始时记录起点，并应用当前频率模式。 */
	virtual void BeginPlay() override;

	/** 注册 FrequencyMode 的复制规则，让客户端能显示正确颜色。 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Tick 中只让服务器移动 Actor。 */
	virtual void Tick(float DeltaSeconds) override;

	/** 运行时切换网络更新频率模式。 */
	UFUNCTION(BlueprintCallable, Category = "Network Lesson 21")
	void SetFrequencyMode(ENetLesson21FrequencyMode NewMode);

	/** 由 Spawner 配置演示参数，方便批量生成高频组和低频组。 */
	void ConfigureDemo(ENetLesson21FrequencyMode NewMode, float NewMoveDistance, float NewMoveSpeed, float NewPhaseOffset);

	/** 返回当前频率模式，供调试和生成器读取。 */
	ENetLesson21FrequencyMode GetFrequencyMode() const { return FrequencyMode; }

protected:
	/** 课程演示用的静态网格组件，默认使用引擎 Cube。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Lesson 21")
	UStaticMeshComponent* MeshComponent;

	/** 当前网络更新频率模式。 */
	UPROPERTY(ReplicatedUsing = OnRep_FrequencyMode, EditAnywhere, BlueprintReadOnly, Category = "Network Lesson 21")
	ENetLesson21FrequencyMode FrequencyMode = ENetLesson21FrequencyMode::HighFrequency;

	/** 从起点沿 X 轴来回移动的最大距离。 */
	UPROPERTY(EditAnywhere, Category = "Network Lesson 21", meta = (ClampMin = "1.0"))
	float MoveDistance = 900.f;

	/** 移动速度，速度越快低频组越容易看出跳变。 */
	UPROPERTY(EditAnywhere, Category = "Network Lesson 21", meta = (ClampMin = "1.0"))
	float MoveSpeed = 650.f;

	/** 批量生成时的相位偏移，避免所有 Actor 完全重叠移动。 */
	UPROPERTY(EditAnywhere, Category = "Network Lesson 21")
	float PhaseOffset = 0.f;

private:
	/** 服务器端记录的移动起点。 */
	FVector StartLocation = FVector::ZeroVector;

	/** 服务器端累计运行时间，用于计算往返位置。 */
	float RunningTime = 0.f;

	/** 根据当前 FrequencyMode 设置 NetUpdateFrequency 和 MinNetUpdateFrequency。 */
	void ApplyFrequencyMode();

	/** 服务器端执行的 X 轴往返移动逻辑。 */
	void MoveActor(float DeltaSeconds);

	/** 按频率模式更新大小和颜色，避免大量文字遮挡画面。 */
	void UpdateDebugVisual();

	/** 客户端收到频率模式后刷新显示。 */
	UFUNCTION()
	void OnRep_FrequencyMode();
};
