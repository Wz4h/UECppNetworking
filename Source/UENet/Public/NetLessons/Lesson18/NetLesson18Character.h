// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NetLesson18Character.generated.h"

class UCameraComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

/**
 * Lesson18 使用的网络角色。
 * 后续网络优化课程复用这个角色，因此这里提供默认飞行移动，方便观察不同网络同步 Demo。
 */
UCLASS()
class UENET_API ANetLesson18Character : public ACharacter
{
	GENERATED_BODY()

public:
	/** 构造函数：创建相机、演示 Mesh 和调试文字，并配置网络移动。 */
	ANetLesson18Character();

	/** 游戏开始时进入飞行移动模式并刷新调试文字。 */
	virtual void BeginPlay() override;

	/** 每帧刷新角色网络状态文字。 */
	virtual void Tick(float DeltaSeconds) override;

	/** 绑定鼠标、WASD 和飞行上下移动输入。 */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	/** 第一人称观察相机。 */
	UPROPERTY(VisibleAnywhere, Category = "Net Lesson 18")
	UCameraComponent* Camera;

	/** 用于表现角色位置和命中体积的演示 Mesh。 */
	UPROPERTY(VisibleAnywhere, Category = "Net Lesson 18")
	UStaticMeshComponent* DemoMesh;

	/** 显示服务器 / 客户端角色网络状态的文字组件。 */
	UPROPERTY(VisibleAnywhere, Category = "Net Lesson 18")
	UTextRenderComponent* DebugText;

	/** 地面行走速度。 */
	UPROPERTY(EditAnywhere, Category = "Net Lesson 18|Movement")
	float WalkSpeed = 450.f;

	/** 飞行移动速度，19-22 课观察网络 Demo 时会用到。 */
	UPROPERTY(EditAnywhere, Category = "Net Lesson 18|Movement")
	float FlySpeed = 900.f;

	/** 服务器射线最大距离。 */
	UPROPERTY(EditAnywhere, Category = "Lag Compensation Demo")
	float FireRange = 10000.f;

	/** 命中伤害数值，本步骤只打印，不做真实伤害系统。 */
	UPROPERTY(EditAnywhere, Category = "Lag Compensation Demo")
	float HitDamage = 10.f;

	/** 是否绘制服务器实际使用的射线。 */
	UPROPERTY(EditAnywhere, Category = "Lag Compensation Demo")
	bool bDrawDebugTrace = true;

	/** 鼠标左键开火输入。 */
	void Input_Fire();

	/** W 键向前移动。 */
	void Input_MoveForward(float Value);

	/** S 键向后移动。 */
	void Input_MoveBackward(float Value);

	/** D 键向右移动。 */
	void Input_MoveRight(float Value);

	/** A 键向左移动。 */
	void Input_MoveLeft(float Value);

	/** Space 键向上飞行。 */
	void Input_MoveUp(float Value);

	/** LeftCtrl 键向下飞行。 */
	void Input_MoveDown(float Value);

	/** 鼠标 X 轴转向。 */
	void Input_Turn(float Value);

	/** 鼠标 Y 轴抬头 / 低头。 */
	void Input_LookUp(float Value);

	/** 客户端请求服务器执行射线检测。 */
	UFUNCTION(Server, Reliable)
	void ServerFire(FVector_NetQuantize TraceStart, FRotator TraceRotation);

	/** 服务器用当前世界位置做命中判定，不做延迟补偿。 */
	void PerformServerTrace(const FVector& TraceStart, const FRotator& TraceRotation);

	/** 刷新头顶调试文字。 */
	void UpdateDebugText();

	/** 输出开火结果到日志和屏幕。 */
	void ShowFireMessage(const FString& Message, const FColor& Color = FColor::Yellow) const;

	/** 把网络角色枚举转成可读文字。 */
	FString NetRoleToString(ENetRole InRole) const;
};
