// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson2/NetLesson2GameMode.h"

#include "NetLessons/Lesson2/NetLesson2Character.h"

ANetLesson2GameMode::ANetLesson2GameMode()
{
	DefaultPawnClass = ANetLesson2Character::StaticClass();
}
