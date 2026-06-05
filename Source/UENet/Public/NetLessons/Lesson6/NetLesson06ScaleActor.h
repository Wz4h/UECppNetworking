// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetLesson06ScaleActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class UENET_API ANetLesson06ScaleActor : public AActor
{
	GENERATED_BODY()

public:
	ANetLesson06ScaleActor();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 服务器修改复制变量，并刷新服务器上的表现。
	void AddScaleValueOnServer();

protected:
	// 演示用盒子。
	UPROPERTY(VisibleAnywhere, Category = "Network Lesson 06")
	UStaticMeshComponent* DemoBox;

	// 服务器权威数值，会同步到客户端。
	UPROPERTY(ReplicatedUsing = OnRep_ScaleValue, VisibleAnywhere, Category = "Network Lesson 06")
	float ScaleValue = 1.f;

	// 客户端收到服务器同步后刷新盒子大小。
	UFUNCTION()
	void OnRep_ScaleValue();

	// 根据当前 ScaleValue 刷新盒子大小。
	void UpdateBoxScale();

	// 打印当前网络身份和数值。
	void PrintNetworkDebugInfo(const FString& Prefix) const;

	// 将 ENetRole 转换为可读字符串，方便日志观察。
	static FString NetRoleToString(ENetRole Role);
};
