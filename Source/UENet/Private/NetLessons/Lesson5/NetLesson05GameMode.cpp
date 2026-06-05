// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson5/NetLesson05GameMode.h"

#include "NetLessons/Lesson5/NetLesson05Character.h"

ANetLesson05GameMode::ANetLesson05GameMode()
{
	DefaultPawnClass = ANetLesson05Character::StaticClass();
}
