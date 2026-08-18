// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "GASLesson2AttackHealMMC.generated.h"

/** 演示 MMC：本次治疗量 = 来源 Attack * 0.5。 */
UCLASS()
class UENET_API UGASLesson2AttackHealMMC : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UGASLesson2AttackHealMMC();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
