// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson22/NetLesson22GameMode.h"

#include "NetLessons/Lesson18/NetLesson18Character.h"

ANetLesson22GameMode::ANetLesson22GameMode()
{
	DefaultPawnClass = ANetLesson18Character::StaticClass();
}
