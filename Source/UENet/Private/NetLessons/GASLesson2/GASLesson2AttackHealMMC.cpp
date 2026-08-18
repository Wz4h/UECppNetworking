// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/GASLesson2/GASLesson2AttackHealMMC.h"

#include "GameplayEffectExecutionCalculation.h"
#include "NetLessons/GASLesson2/GASLesson2AttributeSet.h"

namespace GASLesson2AttackHeal
{
	struct FCaptures
	{
		// 这些宏只生成 AttackDef 和它需要的初始化代码，避免手写容易出错的捕获定义。
		DECLARE_ATTRIBUTE_CAPTUREDEF(Attack)

		FCaptures()
		{
			// Source 表示读取施加效果一方的 Attack；true 表示创建 Spec 时拍快照。
			DEFINE_ATTRIBUTE_CAPTUREDEF(UGASLesson2AttributeSet, Attack, Source, true)
		}
	};

	const FCaptures& Captures()
	{
		static FCaptures Captures;
		return Captures;
	}
}

UGASLesson2AttackHealMMC::UGASLesson2AttackHealMMC()
{
	RelevantAttributesToCapture.Add(GASLesson2AttackHeal::Captures().AttackDef);
}

float UGASLesson2AttackHealMMC::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	float SourceAttack = 0.f;
	GetCapturedAttributeMagnitude(
		GASLesson2AttackHeal::Captures().AttackDef,
		Spec,
		EvaluationParameters,
		SourceAttack);

	return FMath::Max(SourceAttack, 0.f) * 0.5f;
}
