// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NetLesson06Character.generated.h"

UCLASS()
class UENET_API ANetLesson06Character : public ACharacter
{
	GENERATED_BODY()

public:
	ANetLesson06Character();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// 客户端按键入口：请求服务器修改复制变量。
	void TestChangeScaleValue();

	// Server RPC：客户端请求服务器修改物体数值。
	UFUNCTION(Server, Reliable)
	void ServerChangeScaleValue();

	// 服务器查找演示 Actor 并修改数值。
	void ChangeScaleValueOnServer();

	// 打印当前网络身份和 Role。
	void PrintNetworkDebugInfo(const FString& Prefix) const;

	// 将 ENetRole 转换为可读字符串，方便日志观察。
	static FString NetRoleToString(ENetRole Role);
};
