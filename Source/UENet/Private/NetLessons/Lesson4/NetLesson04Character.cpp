// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson4/NetLesson04Character.h"

#include "EngineUtils.h"
#include "InputCoreTypes.h"
#include "NetLessons/Lesson4/NetLesson04ScaleActor.h"

ANetLesson04Character::ANetLesson04Character()
{
	bReplicates = true;
}

void ANetLesson04Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindKey(
		EKeys::F,
		IE_Pressed,
		this,
		&ThisClass::TestChangeScaleValues
	);
}

void ANetLesson04Character::TestChangeScaleValues()
{
	for (TActorIterator<ANetLesson04ScaleActor> It(GetWorld()); It; ++It)
	{
		It->ChangeValuesOnServer();
	}
}
