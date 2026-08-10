// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/GASLesson2/GASLesson2AttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UGASLesson2AttributeSet::UGASLesson2AttributeSet()
{
	// 本课聚焦 GameplayEffect，因此直接使用固定初始值，避免引入初始化 GE 或数据资产。
	InitMaxHealth(100.f);
	InitHealth(100.f);
	InitAttack(20.f);
}

void UGASLesson2AttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UGASLesson2AttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASLesson2AttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASLesson2AttributeSet, Attack, COND_None, REPNOTIFY_Always);
}

void UGASLesson2AttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
}

void UGASLesson2AttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// Instant 和 Periodic GE 执行后再次收口，确保生命值不会越界。
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
}

void UGASLesson2AttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASLesson2AttributeSet, Health, OldHealth);
}

void UGASLesson2AttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASLesson2AttributeSet, MaxHealth, OldMaxHealth);
}

void UGASLesson2AttributeSet::OnRep_Attack(const FGameplayAttributeData& OldAttack)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASLesson2AttributeSet, Attack, OldAttack);
}

