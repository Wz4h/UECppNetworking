// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NetLesson03Character.generated.h"

UCLASS()
class UENET_API ANetLesson03Character : public ACharacter
{
	GENERATED_BODY()

public:
	ANetLesson03Character();

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// 第三课测试用生命值。
	// 注意：客户端可以本地改这份副本，但服务器同步会覆盖客户端错误状态。
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Network Lesson 03")
	float Health = 100.f;

	// 记录初始 Mesh 缩放，方便根据当前 Health 表现体积变化。
	FVector InitialMeshScale = FVector::OneVector;

	// 扣血测试：客户端只改本地副本，服务器刷新所有权威副本。
	void TestServerDamage();

	// 客户端收到服务器同步的 Health 后，自动修正体型表现。
	UFUNCTION()
	void OnRep_Health();

	// 根据当前 Health 刷新 Mesh 体积表现。
	void UpdateMeshScaleByValue();

	// 打印当前网络身份、Role 和生命值。
	void PrintNetworkDebugInfo(const FString& Prefix) const;

	// 将 ENetRole 转换为可读字符串，方便日志观察。
	static FString NetRoleToString(ENetRole Role);
};
