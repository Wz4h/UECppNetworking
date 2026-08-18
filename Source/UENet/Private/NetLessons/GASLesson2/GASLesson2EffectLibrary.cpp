// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/GASLesson2/GASLesson2EffectLibrary.h"

#include "GameplayEffect.h"
#include "Curves/RichCurve.h"
#include "Engine/CurveTable.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "GameplayEffectComponents/TargetTagRequirementsGameplayEffectComponent.h"
#include "GameplayEffectComponents/AssetTagsGameplayEffectComponent.h"
#include "GameplayEffectComponents/ImmunityGameplayEffectComponent.h"
#include "GameplayEffectComponents/RemoveOtherGameplayEffectComponent.h"
#include "NetLessons/GASLesson2/GASLesson2AttackHealMMC.h"
#include "NetLessons/GASLesson2/GASLesson2AttributeSet.h"
#include "NetLessons/GASLesson2/GASLesson2DamageExecution.h"

namespace GASLesson2EffectLibrary
{
	const FName LevelDamageRowName(TEXT("Damage"));
}

void UGASLesson2EffectLibrary::ConfigureGrantedTags(UGameplayEffect* GameplayEffect, const FGameplayTagContainer& GrantedTags)
{
	if (!GameplayEffect)
	{
		return;
	}

	GameplayEffect->Modify();

	FInheritedTagContainer TagChanges;
	TagChanges.Added = GrantedTags;
	TagChanges.UpdateInheritedTagProperties(nullptr);

	UTargetTagsGameplayEffectComponent& TargetTagsComponent =
		GameplayEffect->FindOrAddComponent<UTargetTagsGameplayEffectComponent>();
	TargetTagsComponent.SetAndApplyTargetTagChanges(TagChanges);
}

bool UGASLesson2EffectLibrary::HasGrantedTag(const UGameplayEffect* GameplayEffect, FGameplayTag GrantedTag)
{
	return GameplayEffect && GameplayEffect->GetGrantedTags().HasTagExact(GrantedTag);
}

void UGASLesson2EffectLibrary::ConfigureApplicationRequiredTags(
	UGameplayEffect* GameplayEffect,
	const FGameplayTagContainer& RequiredTags)
{
	if (!GameplayEffect)
	{
		return;
	}

	GameplayEffect->Modify();
	UTargetTagRequirementsGameplayEffectComponent& RequirementsComponent =
		GameplayEffect->FindOrAddComponent<UTargetTagRequirementsGameplayEffectComponent>();
	RequirementsComponent.ApplicationTagRequirements.RequireTags = RequiredTags;
	RequirementsComponent.ApplicationTagRequirements.IgnoreTags.Reset();
}

bool UGASLesson2EffectLibrary::HasApplicationRequiredTag(
	const UGameplayEffect* GameplayEffect,
	FGameplayTag RequiredTag)
{
	const UTargetTagRequirementsGameplayEffectComponent* RequirementsComponent = GameplayEffect
		? GameplayEffect->FindComponent<UTargetTagRequirementsGameplayEffectComponent>()
		: nullptr;
	return RequirementsComponent
		&& RequirementsComponent->ApplicationTagRequirements.RequireTags.HasTagExact(RequiredTag);
}

void UGASLesson2EffectLibrary::ConfigureAssetTags(
	UGameplayEffect* GameplayEffect,
	const FGameplayTagContainer& AssetTags)
{
	if (!GameplayEffect)
	{
		return;
	}

	GameplayEffect->Modify();
	FInheritedTagContainer TagChanges;
	TagChanges.Added = AssetTags;
	TagChanges.UpdateInheritedTagProperties(nullptr);

	UAssetTagsGameplayEffectComponent& AssetTagsComponent =
		GameplayEffect->FindOrAddComponent<UAssetTagsGameplayEffectComponent>();
	AssetTagsComponent.SetAndApplyAssetTagChanges(TagChanges);
}

bool UGASLesson2EffectLibrary::HasAssetTag(const UGameplayEffect* GameplayEffect, FGameplayTag AssetTag)
{
	return GameplayEffect && GameplayEffect->GetAssetTags().HasTagExact(AssetTag);
}

void UGASLesson2EffectLibrary::ConfigureRemoveEffectsWithAssetTags(
	UGameplayEffect* GameplayEffect,
	const FGameplayTagContainer& AssetTags)
{
	if (!GameplayEffect)
	{
		return;
	}

	GameplayEffect->Modify();
	URemoveOtherGameplayEffectComponent& RemoveComponent =
		GameplayEffect->FindOrAddComponent<URemoveOtherGameplayEffectComponent>();
	RemoveComponent.RemoveGameplayEffectQueries = {
		FGameplayEffectQuery::MakeQuery_MatchAnyEffectTags(AssetTags)
	};
}

void UGASLesson2EffectLibrary::ConfigureImmunityToAssetTags(
	UGameplayEffect* GameplayEffect,
	const FGameplayTagContainer& AssetTags)
{
	if (!GameplayEffect)
	{
		return;
	}

	GameplayEffect->Modify();
	UImmunityGameplayEffectComponent& ImmunityComponent =
		GameplayEffect->FindOrAddComponent<UImmunityGameplayEffectComponent>();
	ImmunityComponent.ImmunityQueries = {
		FGameplayEffectQuery::MakeQuery_MatchAnyEffectTags(AssetTags)
	};
}

bool UGASLesson2EffectLibrary::HasRemoveEffectsQueryForAssetTag(
	const UGameplayEffect* GameplayEffect,
	FGameplayTag AssetTag)
{
	const URemoveOtherGameplayEffectComponent* RemoveComponent = GameplayEffect
		? GameplayEffect->FindComponent<URemoveOtherGameplayEffectComponent>()
		: nullptr;
	if (!RemoveComponent)
	{
		return false;
	}

	FGameplayTagContainer TestTags;
	TestTags.AddTag(AssetTag);
	for (const FGameplayEffectQuery& Query : RemoveComponent->RemoveGameplayEffectQueries)
	{
		if (Query.EffectTagQuery.Matches(TestTags))
		{
			return true;
		}
	}
	return false;
}

bool UGASLesson2EffectLibrary::HasImmunityQueryForAssetTag(
	const UGameplayEffect* GameplayEffect,
	FGameplayTag AssetTag)
{
	const UImmunityGameplayEffectComponent* ImmunityComponent = GameplayEffect
		? GameplayEffect->FindComponent<UImmunityGameplayEffectComponent>()
		: nullptr;
	if (!ImmunityComponent)
	{
		return false;
	}

	FGameplayTagContainer TestTags;
	TestTags.AddTag(AssetTag);
	for (const FGameplayEffectQuery& Query : ImmunityComponent->ImmunityQueries)
	{
		if (Query.EffectTagQuery.Matches(TestTags))
		{
			return true;
		}
	}
	return false;
}

void UGASLesson2EffectLibrary::ConfigureAttributeBasedHealModifier(UGameplayEffect* GameplayEffect)
{
	if (!GameplayEffect)
	{
		return;
	}

	FAttributeBasedFloat Magnitude;
	Magnitude.BackingAttribute = FGameplayEffectAttributeCaptureDefinition(
		UGASLesson2AttributeSet::GetMaxHealthAttribute(),
		EGameplayEffectAttributeCaptureSource::Target,
		false);
	Magnitude.Coefficient = FScalableFloat(0.25f);

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UGASLesson2AttributeSet::GetHealthAttribute();
	Modifier.ModifierOp = EGameplayModOp::AddBase;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(Magnitude);

	GameplayEffect->Modify();
	GameplayEffect->Modifiers = {Modifier};
}

void UGASLesson2EffectLibrary::ConfigureMMCHealModifier(UGameplayEffect* GameplayEffect)
{
	if (!GameplayEffect)
	{
		return;
	}

	FCustomCalculationBasedFloat Magnitude;
	Magnitude.CalculationClassMagnitude = UGASLesson2AttackHealMMC::StaticClass();

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UGASLesson2AttributeSet::GetHealthAttribute();
	Modifier.ModifierOp = EGameplayModOp::AddBase;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(Magnitude);

	GameplayEffect->Modify();
	GameplayEffect->Modifiers = {Modifier};
}

void UGASLesson2EffectLibrary::ConfigureDamageExecution(UGameplayEffect* GameplayEffect)
{
	if (!GameplayEffect)
	{
		return;
	}

	FGameplayEffectExecutionDefinition Execution;
	Execution.CalculationClass = UGASLesson2DamageExecution::StaticClass();

	GameplayEffect->Modify();
	GameplayEffect->Modifiers.Reset();
	GameplayEffect->Executions = {Execution};
}

void UGASLesson2EffectLibrary::ConfigureLevelDamageCurve(UCurveTable* CurveTable)
{
	if (!CurveTable)
	{
		return;
	}

	CurveTable->Modify();
	CurveTable->EmptyTable();
	FRichCurve& DamageCurve = CurveTable->AddRichCurve(GASLesson2EffectLibrary::LevelDamageRowName);
	DamageCurve.AddKey(1.f, 10.f);
	DamageCurve.AddKey(2.f, 20.f);
	DamageCurve.AddKey(3.f, 40.f);
	CurveTable->MarkPackageDirty();
}

void UGASLesson2EffectLibrary::ConfigureLevelDamageModifier(
	UGameplayEffect* GameplayEffect,
	UCurveTable* CurveTable)
{
	if (!GameplayEffect || !CurveTable)
	{
		return;
	}

	FScalableFloat LevelDamageMagnitude;
	LevelDamageMagnitude.SetScalingValue(-1.f, GASLesson2EffectLibrary::LevelDamageRowName, CurveTable);

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UGASLesson2AttributeSet::GetHealthAttribute();
	Modifier.ModifierOp = EGameplayModOp::AddBase;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(LevelDamageMagnitude);

	GameplayEffect->Modify();
	GameplayEffect->Modifiers = {Modifier};
}

float UGASLesson2EffectLibrary::GetLevelDamageMagnitude(const UCurveTable* CurveTable, float Level)
{
	if (!CurveTable)
	{
		return 0.f;
	}

	FScalableFloat LevelDamageMagnitude;
	LevelDamageMagnitude.SetScalingValue(
		-1.f,
		GASLesson2EffectLibrary::LevelDamageRowName,
		const_cast<UCurveTable*>(CurveTable));
	return LevelDamageMagnitude.GetValueAtLevel(Level);
}
