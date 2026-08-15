// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/GASLesson2/GASLesson2EffectLibrary.h"

#include "GameplayEffect.h"
#include "Curves/RichCurve.h"
#include "Engine/CurveTable.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "GameplayEffectComponents/TargetTagRequirementsGameplayEffectComponent.h"
#include "NetLessons/GASLesson2/GASLesson2AttributeSet.h"

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
