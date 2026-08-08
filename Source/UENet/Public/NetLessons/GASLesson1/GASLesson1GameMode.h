// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GASLesson1GameMode.generated.h"

/**
 * GASLesson1 的 GameMode。
 * 用于在 World Settings 中切换到 AttributeSet 课程角色。
 */
UCLASS()
class UENET_API AGASLesson1GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	/** 构造函数：设置本课程默认 Pawn。 */
	AGASLesson1GameMode();
};
