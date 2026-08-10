// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameFramework/Character.h"
#include "GameplayEffectTypes.h"
#include "GASLesson2Character.generated.h"

class UAbilitySystemComponent;
class UCameraComponent;
class UGameplayEffect;
class UGASLesson2AttributeSet;
class UStaticMeshComponent;

/** 使用数字键演示 GameplayEffect 生命周期和 Modifier 行为。 */
UCLASS()
class UENET_API AGASLesson2Character : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AGASLesson2Character();

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS Lesson 2")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS Lesson 2")
	TObjectPtr<UGASLesson2AttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, Category = "GAS Lesson 2")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, Category = "GAS Lesson 2")
	TObjectPtr<UStaticMeshComponent> DemoMesh;

	UPROPERTY(EditDefaultsOnly, Category = "GAS Lesson 2|Effects")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS Lesson 2|Effects")
	TSubclassOf<UGameplayEffect> HealEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS Lesson 2|Effects")
	TSubclassOf<UGameplayEffect> AttackBuffEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS Lesson 2|Effects")
	TSubclassOf<UGameplayEffect> WeaponEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS Lesson 2|Effects")
	TSubclassOf<UGameplayEffect> DynamicDamageEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS Lesson 2|Effects")
	TSubclassOf<UGameplayEffect> PoisonEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS Lesson 2|Effects")
	TSubclassOf<UGameplayEffect> StackAttackEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS Lesson 2|Debug", meta = (ClampMin = "0.0"))
	float DynamicDamageAmount = 30.f;

private:
	FActiveGameplayEffectHandle WeaponEffectHandle;

	void InitializeAbilitySystem();
	void BindAttributeDelegates();

	FActiveGameplayEffectHandle ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level = 1.f);
	void ApplyDynamicDamage(float Damage);
	void RemoveWeaponEffect();
	void ClearDemoEffects();
	void PrintAttributes() const;

	void OnHealthChanged(const FOnAttributeChangeData& Data);
	void OnAttackChanged(const FOnAttributeChangeData& Data);

	void RequestTest(int32 TestIndex);
	void ExecuteTest(int32 TestIndex);
	void TestDamage();
	void TestHeal();
	void TestAttackBuff();
	void TestEquipWeapon();
	void TestRemoveWeapon();
	void TestDynamicDamage();
	void TestPoison();
	void TestStackAttack();
	void TestPrintAttributes();
	void TestClearEffects();

	UFUNCTION(Server, Reliable)
	void ServerApplyTestEffect(int32 TestIndex);

	void ShowDebugMessage(const FString& Message, const FColor& Color = FColor::Cyan) const;
};

