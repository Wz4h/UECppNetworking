// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InteractionDemoCharacter.generated.h"

class UInteractionComponent;

// 联机交互演示角色，用于展示如何挂载交互组件并绑定按键调用交互流程。
UCLASS(Blueprintable)
class ONLINEINTERACTION_API AInteractionDemoCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// 创建演示角色，并默认挂载 UInteractionComponent。
	AInteractionDemoCharacter();

	// 每帧可选更新当前可交互目标，便于课程演示对准即可交互。
	virtual void Tick(float DeltaSeconds) override;

	// 绑定 E 键到交互请求。
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// 角色持有的通用交互组件；真正的联机交互请求由该组件发送到服务器。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction")
	TObjectPtr<UInteractionComponent> InteractionComponent;

	// 是否在 Tick 中持续更新当前目标；正式项目可关闭后改为 Timer 或低频刷新。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	bool bUpdateTargetEveryTick = true;

	// E 键按下时调用，先刷新目标，再请求交互。
	void HandleInteractPressed();
};
