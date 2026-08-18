// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/GASLesson2/GASLesson2DamageExecution.h"

#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "NetLessons/GASLesson2/GASLesson2AttributeSet.h"

namespace GASLesson2DamageExecution
{
	const FName BaseDamageTagName(TEXT("Data.BaseDamage"));

	struct FCaptures
	{
		DECLARE_ATTRIBUTE_CAPTUREDEF(Attack)
		DECLARE_ATTRIBUTE_CAPTUREDEF(Defense)

		FCaptures()
		{
			// Attack 在 Spec 创建时拍快照；Defense 在命中执行时读取目标的当前值。
			DEFINE_ATTRIBUTE_CAPTUREDEF(UGASLesson2AttributeSet, Attack, Source, true)
			DEFINE_ATTRIBUTE_CAPTUREDEF(UGASLesson2AttributeSet, Defense, Target, false)
		}
	};

	const FCaptures& Captures()
	{
		static FCaptures Captures;
		return Captures;
	}
}

UGASLesson2DamageExecution::UGASLesson2DamageExecution()
{
	RelevantAttributesToCapture.Add(GASLesson2DamageExecution::Captures().AttackDef);
	RelevantAttributesToCapture.Add(GASLesson2DamageExecution::Captures().DefenseDef);
}

void UGASLesson2DamageExecution::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	float SourceAttack = 0.f;
	float TargetDefense = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		GASLesson2DamageExecution::Captures().AttackDef,
		EvaluationParameters,
		SourceAttack);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		GASLesson2DamageExecution::Captures().DefenseDef,
		EvaluationParameters,
		TargetDefense);

	const FGameplayTag BaseDamageTag = FGameplayTag::RequestGameplayTag(
		GASLesson2DamageExecution::BaseDamageTagName,
		false);
	const float BaseDamage = BaseDamageTag.IsValid()
		? Spec.GetSetByCallerMagnitude(BaseDamageTag, false, 0.f)
		: 0.f;
	const float FinalDamage = FMath::Max(BaseDamage + SourceAttack - TargetDefense, 0.f);

	UE_LOG(LogTemp, Warning,
		TEXT("[GAS] Execution Damage: Base %.2f + Attack %.2f - Defense %.2f = %.2f"),
		BaseDamage, SourceAttack, TargetDefense, FinalDamage);

	// Execution 不直接改 Health，而是把结果交给 Damage Meta Attribute 统一结算。
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
		UGASLesson2AttributeSet::GetDamageAttribute(),
		EGameplayModOp::Additive,
		FinalDamage));
}
