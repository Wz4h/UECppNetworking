// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GASLesson2AttributeSet.generated.h"

#define GASLESSON2_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/** GameplayEffect 课程使用的最小属性集合。 */
UCLASS()
class UENET_API UGASLesson2AttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UGASLesson2AttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "GAS Lesson 2|Attribute")
	FGameplayAttributeData Health;
	GASLESSON2_ATTRIBUTE_ACCESSORS(UGASLesson2AttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "GAS Lesson 2|Attribute")
	FGameplayAttributeData MaxHealth;
	GASLESSON2_ATTRIBUTE_ACCESSORS(UGASLesson2AttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Attack, Category = "GAS Lesson 2|Attribute")
	FGameplayAttributeData Attack;
	GASLESSON2_ATTRIBUTE_ACCESSORS(UGASLesson2AttributeSet, Attack)

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	void OnRep_Attack(const FGameplayAttributeData& OldAttack);
};

