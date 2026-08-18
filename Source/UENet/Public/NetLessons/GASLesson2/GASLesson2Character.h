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

	UPROPERTY(EditDefaultsOnly, Category = "GAS Lesson 2|Effects")
	TSubclassOf<UGameplayEffect> AttackMultiplyEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS Lesson 2|Effects")
	TSubclassOf<UGameplayEffect> LevelDamageEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS Lesson 2|Effects")
	TSubclassOf<UGameplayEffect> OiledEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS Lesson 2|Effects")
	TSubclassOf<UGameplayEffect> BurningRequiresOilEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS Lesson 2|Effects")
	TSubclassOf<UGameplayEffect> AttributeBasedHealEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS Lesson 2|Effects")
	TSubclassOf<UGameplayEffect> MMCHealEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS Lesson 2|Effects")
	TSubclassOf<UGameplayEffect> ExecutionDamageEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS Lesson 2|Effects")
	TSubclassOf<UGameplayEffect> CleanseEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS Lesson 2|Effects")
	TSubclassOf<UGameplayEffect> DamageImmunityEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS Lesson 2|Debug", meta = (ClampMin = "0.0"))
	float DynamicDamageAmount = 30.f;

	UPROPERTY(EditDefaultsOnly, Category = "GAS Lesson 2|Debug", meta = (ClampMin = "0.0"))
	float ExecutionBaseDamage = 30.f;

	UPROPERTY(EditDefaultsOnly, Category = "GAS Lesson 2|Debug", meta = (ClampMin = "0.0"))
	float TargetEffectRange = 1500.f;

private:
	FActiveGameplayEffectHandle WeaponEffectHandle;
	int32 NextLevelDamageTestLevel = 1;

	void InitializeAbilitySystem();
	void BindAttributeDelegates();

	/** 按照 Context -> Spec -> Apply 的标准流程，把一个 GameplayEffect 应用给自己的 ASC。 */
	FActiveGameplayEffectHandle ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, float EffectLevel = 1.f);
	/** 由自己的 ASC 创建 Spec，再把 GameplayEffect 应用给另一个目标 ASC。 */
	FActiveGameplayEffectHandle ApplyEffectToTarget(TSubclassOf<UGameplayEffect> EffectClass, UAbilitySystemComponent* TargetAbilitySystem, float EffectLevel = 1.f);
	void ApplyDynamicDamage(float Damage);
	bool FindAimedTarget(FHitResult& OutHitResult, UAbilitySystemComponent*& OutTargetAbilitySystem) const;
	void ApplyDamageToAimedTarget();
	void ApplyExecutionDamageToAimedTarget();
	void RemoveWeaponEffect();
	void ClearDemoEffects();
	void PrintAttributes() const;

	void OnHealthChanged(const FOnAttributeChangeData& Data);
	void OnAttackChanged(const FOnAttributeChangeData& Data);
	void OnDefenseChanged(const FOnAttributeChangeData& Data);

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
	void TestAttackMultiply();
	void TestLevelDamage();
	void TestApplyOiled();
	void TestBurningRequiresOil();
	void TestDamageAimedTarget();
	void TestAttributeBasedHeal();
	void TestMMCHeal();
	void TestExecutionDamageAimedTarget();
	void TestCleanse();
	void TestDamageImmunity();

	UFUNCTION(Server, Reliable)
	void ServerApplyTestEffect(int32 TestIndex);

	void ShowDebugMessage(const FString& Message, const FColor& Color = FColor::Cyan) const;
};
