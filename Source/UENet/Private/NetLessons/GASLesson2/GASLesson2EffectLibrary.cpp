// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/GASLesson2/GASLesson2EffectLibrary.h"

#include "GameplayEffect.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

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
