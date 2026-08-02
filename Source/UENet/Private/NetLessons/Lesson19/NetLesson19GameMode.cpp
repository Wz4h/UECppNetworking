// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson19/NetLesson19GameMode.h"

#include "NetLessons/Lesson18/NetLesson18Character.h"

ANetLesson19GameMode::ANetLesson19GameMode()
{
	DefaultPawnClass = ANetLesson18Character::StaticClass();
}
