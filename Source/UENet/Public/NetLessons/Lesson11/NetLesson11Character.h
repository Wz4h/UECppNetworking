// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NetLesson11Character.generated.h"

class UTextRenderComponent;

UCLASS()
class UENET_API ANetLesson11Character : public ACharacter
{
	GENERATED_BODY()

public:
	ANetLesson11Character();

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Network Lesson 11")
	UTextRenderComponent* FrameworkText;

	// F：客户端通过自己拥有的 Character 请求服务器修改 Box。
	void RequestChangeBox();

	// G：客户端通过自己拥有的 Character 请求服务器切换 Coin 相关性。
	void RequestToggleCoinRelevancy();

	// H：客户端通过自己拥有的 Character 请求服务器销毁 Door。
	void RequestDestroyDoor();

	UFUNCTION(Server, Reliable)
	void ServerChangeBox();

	UFUNCTION(Server, Reliable)
	void ServerToggleCoinRelevancy();

	UFUNCTION(Server, Reliable)
	void ServerDestroyDoor();

	void UpdateFrameworkText();
};
