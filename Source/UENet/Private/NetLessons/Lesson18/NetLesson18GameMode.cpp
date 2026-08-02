// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson18/NetLesson18GameMode.h"

#include "NetLessons/Lesson18/NetLesson18Character.h"

ANetLesson18GameMode::ANetLesson18GameMode()
{
	DefaultPawnClass = ANetLesson18Character::StaticClass();
}
