// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NetLesson2Character.generated.h"

class IRepChangedPropertyTracker;

// 第二课实验开关：
// false = 实验A：Health 不同步，服务器修改后客户端不会变化。
// true = 实验B：Health 同步，服务器修改后客户端会变化。
#define NET_LESSON2_ENABLE_HEALTH_SYNC true

UCLASS()
class UENET_API ANetLesson2Character : public ACharacter
{
	GENERATED_BODY()

public:
	ANetLesson2Character();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// 第二课测试用生命值。
	// 实验A 将 NET_LESSON2_ENABLE_HEALTH_SYNC 改为 false，观察客户端不会同步。
	// 实验B 将 NET_LESSON2_ENABLE_HEALTH_SYNC 改为 true，观察客户端会同步。
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Network Lesson 2")
	float Health = 100.f;

	// 记录初始 Mesh 缩放，方便根据当前 Health 表现体积变化。
	FVector InitialMeshScale = FVector::OneVector;

	// 服务器扣血测试：只有权威端可以修改 Health。
	void TestServerDamage();

	// 根据当前 Health 刷新 Mesh 体积表现。
	void UpdateMeshScaleByValue();

	// 打印当前网络身份、Role 和生命值。
	void PrintNetworkDebugInfo(const FString& Prefix) const;

	// 将 ENetRole 转换为可读字符串，方便日志观察。
	static FString NetRoleToString(ENetRole Role);
};
