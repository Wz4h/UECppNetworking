// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson1/NetLesson1GameMode.h"

#include "NetLessons/Lesson1/NetLesson1Character.h"

ANetLesson1GameMode::ANetLesson1GameMode()
{
	DefaultPawnClass = ANetLesson1Character::StaticClass();
}
