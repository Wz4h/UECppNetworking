// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NetLesson04Character.generated.h"

UCLASS()
class UENET_API ANetLesson04Character : public ACharacter
{
	GENERATED_BODY()

public:
	ANetLesson04Character();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// 触发第四课 Actor 修改变量。
	void TestChangeScaleValues();
};
