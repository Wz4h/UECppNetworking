// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/GASLesson1/GASLesson1GameMode.h"

#include "NetLessons/GASLesson1/GASLesson1Character.h"

AGASLesson1GameMode::AGASLesson1GameMode()
{
	DefaultPawnClass = AGASLesson1Character::StaticClass();
}
