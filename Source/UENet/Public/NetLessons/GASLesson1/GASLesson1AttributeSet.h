// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GASLesson1AttributeSet.generated.h"

// GAS 官方把一个属性拆成两部分：
// 1. FGameplayAttributeData 变量本身，例如 Health。
// 2. 一组围绕这个变量的访问函数，例如 GetHealth()、SetHealth()、InitHealth()、GetHealthAttribute()。
//
// 这些函数名字都很固定，手写容易重复且出错，所以 GAS 在 AttributeSet.h 里提供了下面四个宏：
// - GAMEPLAYATTRIBUTE_PROPERTY_GETTER：生成 GetHealthAttribute()，返回“Health 这个属性的描述对象”。
// - GAMEPLAYATTRIBUTE_VALUE_GETTER：生成 GetHealth()，读取当前值。
// - GAMEPLAYATTRIBUTE_VALUE_SETTER：生成 SetHealth()，直接设置当前值。
// - GAMEPLAYATTRIBUTE_VALUE_INITTER：生成 InitHealth()，初始化 BaseValue 和 CurrentValue。
//
// 本宏只是把这四个官方宏打包，方便每个属性后面写一行。
#define GASLESSON1_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \ 
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * GASLesson1 的属性集合。
 * 只负责保存、限制和复制属性；初始化和测试修改由 Character 发起。
 */
UCLASS()
class UENET_API UGASLesson1AttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	/** 注册 GAS 属性复制规则。 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 属性被改写前做最大值限制。 */
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	/** 当前生命值。 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "GAS Lesson 1|Attribute")
	FGameplayAttributeData Health;
	// 这一行不会创建新变量，只是为 Health 生成 GAS 常用函数。
	// 后面扣血时用 GetHealthAttribute() 告诉 ASC：“我要修改的是 Health 属性”。
	GASLESSON1_ATTRIBUTE_ACCESSORS(UGASLesson1AttributeSet, Health)

	/** 最大生命值。 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "GAS Lesson 1|Attribute")
	FGameplayAttributeData MaxHealth;
	GASLESSON1_ATTRIBUTE_ACCESSORS(UGASLesson1AttributeSet, MaxHealth)

	/** 当前法力值。 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "GAS Lesson 1|Attribute")
	FGameplayAttributeData Mana;
	GASLESSON1_ATTRIBUTE_ACCESSORS(UGASLesson1AttributeSet, Mana)

	/** 最大法力值。 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "GAS Lesson 1|Attribute")
	FGameplayAttributeData MaxMana;
	GASLESSON1_ATTRIBUTE_ACCESSORS(UGASLesson1AttributeSet, MaxMana)

	/** 攻击力。 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttackPower, Category = "GAS Lesson 1|Attribute")
	FGameplayAttributeData AttackPower;
	GASLESSON1_ATTRIBUTE_ACCESSORS(UGASLesson1AttributeSet, AttackPower)

	/** 防御力。 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Defense, Category = "GAS Lesson 1|Attribute")
	FGameplayAttributeData Defense;
	GASLESSON1_ATTRIBUTE_ACCESSORS(UGASLesson1AttributeSet, Defense)

protected:
	// OnRep 函数会在客户端收到服务器复制的新值时执行。
	// 参数 OldHealth / OldMana 是复制前的旧值，GAS 会用旧值和当前新值广播 Attribute Change Delegate。
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana);

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana);

	UFUNCTION()
	void OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower);

	UFUNCTION()
	void OnRep_Defense(const FGameplayAttributeData& OldDefense);
};
