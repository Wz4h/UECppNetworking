// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NetLesson19GameMode.generated.h"

/**
 * Lesson19 的 GameMode。
 * 用于在 World Settings 中直接切换到 Relevancy 优化课程，并复用现有可移动角色观察远近同步差异。
 */
UCLASS()
class UENET_API ANetLesson19GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	/** 构造函数：设置本课程默认 Pawn。 */
	ANetLesson19GameMode();
};
