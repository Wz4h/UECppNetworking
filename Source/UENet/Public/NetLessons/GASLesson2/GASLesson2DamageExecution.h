// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "GASLesson2DamageExecution.generated.h"

/** 演示 Execution：最终伤害 = SetByCaller 基础伤害 + 来源 Attack - 目标 Defense。 */
UCLASS()
class UENET_API UGASLesson2DamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UGASLesson2DamageExecution();

	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
