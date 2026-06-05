// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson3/NetLesson03GameMode.h"

#include "NetLessons/Lesson3/NetLesson03Character.h"

ANetLesson03GameMode::ANetLesson03GameMode()
{
	DefaultPawnClass = ANetLesson03Character::StaticClass();
}
