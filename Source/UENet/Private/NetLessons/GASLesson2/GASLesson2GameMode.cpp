// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/GASLesson2/GASLesson2GameMode.h"

#include "NetLessons/GASLesson2/GASLesson2Character.h"

AGASLesson2GameMode::AGASLesson2GameMode()
{
	DefaultPawnClass = AGASLesson2Character::StaticClass();
}

