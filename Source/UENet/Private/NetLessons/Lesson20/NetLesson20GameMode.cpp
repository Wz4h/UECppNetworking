// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson20/NetLesson20GameMode.h"

#include "NetLessons/Lesson18/NetLesson18Character.h"

ANetLesson20GameMode::ANetLesson20GameMode()
{
	DefaultPawnClass = ANetLesson18Character::StaticClass();
}
