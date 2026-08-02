// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NetLesson22GameMode.generated.h"

/**
 * Lesson22 的 GameMode。
 * 用于在 World Settings 中直接切换到 FastArray 增量同步课程，并复用现有可移动角色观察日志和网络数据。
 */
UCLASS()
class UENET_API ANetLesson22GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	/** 构造函数：设置本课程默认 Pawn。 */
	ANetLesson22GameMode();
};
