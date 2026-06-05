// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NetLesson05Character.generated.h"

UCLASS()
class UENET_API ANetLesson05Character : public ACharacter
{
	GENERATED_BODY()

public:
	ANetLesson05Character();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// 客户端按键入口：本地调用后请求服务器生成 Actor。
	void TestSpawnActor();

	// Server RPC：客户端请求服务器执行生成 Actor。
	UFUNCTION(Server, Reliable)
	void ServerSpawnActor();

	// 只有服务器真正执行 Spawn。
	void SpawnActorOnServer();

	// 服务器记录已生成数量，用来错开生成位置。
	int32 SpawnedActorCount = 0;

	// 打印当前网络身份和 Role。
	void PrintNetworkDebugInfo(const FString& Prefix) const;

	// 将 ENetRole 转换为可读字符串，方便日志观察。
	static FString NetRoleToString(ENetRole Role);
};
