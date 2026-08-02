// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NetLesson20GameMode.generated.h"

/**
 * Lesson20 的 GameMode。
 * 用于在 World Settings 中直接切换到 Dormancy 优化课程，并复用现有可移动角色触发和观察门同步。
 */
UCLASS()
class UENET_API ANetLesson20GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	/** 构造函数：设置本课程默认 Pawn。 */
	ANetLesson20GameMode();
};
