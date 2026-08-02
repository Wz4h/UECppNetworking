// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetLesson20DormancyDoor.generated.h"

class UStaticMeshComponent;

/**
 * Dormancy 课程用的门 Actor。
 * 门只复制 bIsOpen，一个静止门可以进入 DormantAll，打开前由服务器 FlushNetDormancy 唤醒。
 */
UCLASS()
class UENET_API ANetLesson20DormancyDoor : public AActor
{
	GENERATED_BODY()

public:
	/** 构造函数：创建 Mesh，开启复制，设置默认门体外观。 */
	ANetLesson20DormancyDoor();

	/** 注册 bIsOpen 的复制规则。 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 游戏开始时记录关门旋转，并按配置让服务器门进入休眠。 */
	virtual void BeginPlay() override;

	/** 服务器开门入口；客户端调用会直接返回。 */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Network Lesson 20")
	void OpenDoor();

	/** 设置是否使用 Dormancy，Spawner 生成后会用这个函数统一配置。 */
	void SetUseDormancy(bool bInUseDormancy);

protected:
	/** 课程演示用的门网格，默认用 Cube 拉伸成长方体。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Lesson 20")
	UStaticMeshComponent* MeshComponent;

	/** 门是否已经打开，复制到客户端后驱动视觉旋转。 */
	UPROPERTY(ReplicatedUsing = OnRep_IsOpen, VisibleAnywhere, Category = "Network Lesson 20")
	bool bIsOpen = false;

	/** 是否启用 DormantAll，用于对比普通复制和休眠优化。 */
	UPROPERTY(EditAnywhere, Category = "Network Lesson 20")
	bool bUseDormancy = true;

	/** 关门时的初始旋转。 */
	UPROPERTY(VisibleAnywhere, Category = "Network Lesson 20")
	FRotator ClosedRotation = FRotator::ZeroRotator;

	/** 开门后的目标旋转。 */
	UPROPERTY(VisibleAnywhere, Category = "Network Lesson 20")
	FRotator OpenRotation = FRotator::ZeroRotator;

	/** 客户端收到 bIsOpen 后更新门的视觉状态。 */
	UFUNCTION()
	void OnRep_IsOpen();

	/** 根据 bIsOpen 应用门的旋转。 */
	void ApplyDoorVisualState();
};
