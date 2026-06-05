// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NetLesson1Character.generated.h"

UCLASS()
class UENET_API ANetLesson1Character : public ACharacter
{
	GENERATED_BODY()

public:
	ANetLesson1Character();

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// 本地测试用生命值。
	// 注意：这里故意不启用属性复制，因为第一课要演示“变量不会自动同步”。
	UPROPERTY(VisibleAnywhere, Category = "Network Lesson 1")
	float Health = 100.f;

	// 记录初始 Mesh 缩放，方便本地扣血后只改变当前机器上的 Mesh 表现。
	FVector InitialMeshScale = FVector::OneVector;

	// 本地扣血测试：只修改当前机器上的 Health。
	void TestLocalDamage();

	// 本地 Mesh 受伤表现：只修改当前机器上的 Mesh 缩放。
	void ApplyLocalMeshDamageEffect();

	// 打印当前网络身份、Role 和生命值。
	void PrintNetworkDebugInfo(const FString& Prefix) const;

	// 将 ENetRole 转换为可读字符串，方便日志观察。
	static FString NetRoleToString(ENetRole Role);
};
