// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetLesson04ScaleActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class UENET_API ANetLesson04ScaleActor : public AActor
{
	GENERATED_BODY()

public:
	ANetLesson04ScaleActor();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 服务器修改两个变量：一个会同步，一个不会同步。
	void ChangeValuesOnServer();

protected:
	// 演示用盒子：宽由复制变量控制，高由非复制变量控制。
	UPROPERTY(VisibleAnywhere, Category = "Network Lesson 04")
	UStaticMeshComponent* DemoBox;

	// 会从服务器同步到客户端，控制盒子宽度。
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedWidth, VisibleAnywhere, Category = "Network Lesson 04")
	float ReplicatedWidth = 1.f;

	// 不会从服务器同步到客户端，控制盒子高度。
	UPROPERTY(VisibleAnywhere, Category = "Network Lesson 04")
	float LocalOnlyHeight = 1.f;

	// 客户端收到复制变量后，更新盒子大小。
	UFUNCTION()
	void OnRep_ReplicatedWidth();

	// 按当前变量刷新盒子的宽和高。
	void UpdateBoxScale();

	// 打印当前网络身份和两个变量。
	void PrintNetworkDebugInfo(const FString& Prefix) const;

	// 将 ENetRole 转换为可读字符串，方便日志观察。
	static FString NetRoleToString(ENetRole Role);
};
