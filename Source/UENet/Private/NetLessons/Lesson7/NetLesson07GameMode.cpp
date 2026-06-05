// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson7/NetLesson07GameMode.h"

#include "NetLessons/Lesson7/NetLesson07Character.h"
#include "NetLessons/Lesson7/NetLesson07GameState.h"
#include "NetLessons/Lesson7/NetLesson07PlayerController.h"
#include "NetLessons/Lesson7/NetLesson07PlayerState.h"

ANetLesson07GameMode::ANetLesson07GameMode()
{
	DefaultPawnClass = ANetLesson07Character::StaticClass();
	PlayerControllerClass = ANetLesson07PlayerController::StaticClass();
	GameStateClass = ANetLesson07GameState::StaticClass();
	PlayerStateClass = ANetLesson07PlayerState::StaticClass();
}
