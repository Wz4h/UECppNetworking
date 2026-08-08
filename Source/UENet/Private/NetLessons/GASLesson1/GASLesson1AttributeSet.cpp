// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/GASLesson1/GASLesson1AttributeSet.h"

#include "Net/UnrealNetwork.h"

void UGASLesson1AttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// DOREPLIFETIME_CONDITION_NOTIFY 是 UE 的复制注册宏。
	// 参数含义：
	// - UGASLesson1AttributeSet：这个复制字段属于哪个类。
	// - Health：要复制的属性名。
	// - COND_None：没有额外条件，相关客户端都能收到。
	// - REPNOTIFY_Always：收到复制数据时总是尝试执行 OnRep，课堂 Demo 中更容易观察变化。
	//
	// AttributeSet 本身不是 Actor，但它挂在已复制的 ASC/Actor 上，因此属性仍然可以通过这里注册复制。
	DOREPLIFETIME_CONDITION_NOTIFY(UGASLesson1AttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASLesson1AttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASLesson1AttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASLesson1AttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASLesson1AttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASLesson1AttributeSet, Defense, COND_None, REPNOTIFY_Always);
}

void UGASLesson1AttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.f);

		// 如果最大生命值被调低，当前生命值也不能继续高于新的最大值。
		// 这里演示 AttributeSet 可以维护属性之间的约束关系。
		if (GetHealth() > NewValue)
		{
			SetHealth(NewValue);
		}
	}
	else if (Attribute == GetMaxManaAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.f);

		if (GetMana() > NewValue)
		{
			SetMana(NewValue);
		}
	}
}

void UGASLesson1AttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	// GAMEPLAYATTRIBUTE_REPNOTIFY 是 GAS 的 OnRep 辅助宏。
	// 它会把“旧值 OldHealth”和“当前复制后的新值 Health”交给 ASC，
	// 这样 GetGameplayAttributeValueChangeDelegate() 绑定的监听函数才能在客户端触发。
	// 如果这里只写普通日志，不调用这个宏，GAS 的属性变化监听链路就会断掉。
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASLesson1AttributeSet, Health, OldHealth);
}

void UGASLesson1AttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASLesson1AttributeSet, MaxHealth, OldMaxHealth);
}

void UGASLesson1AttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASLesson1AttributeSet, Mana, OldMana);
}

void UGASLesson1AttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASLesson1AttributeSet, MaxMana, OldMaxMana);
}

void UGASLesson1AttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASLesson1AttributeSet, AttackPower, OldAttackPower);
}

void UGASLesson1AttributeSet::OnRep_Defense(const FGameplayAttributeData& OldDefense)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASLesson1AttributeSet, Defense, OldDefense);
}
