// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "GameplayEffectTypes.h"
#include "GASLesson1Character.generated.h"

class UAbilitySystemComponent;
class UCameraComponent;
class UGASLesson1AttributeSet;
class UStaticMeshComponent;
class UTextRenderComponent;

/**
 * GASLesson1 测试角色。
 * 拥有 ASC 和 AttributeSet，按 F 请求服务器扣除 Health，用于观察属性复制和变化监听。
 */
UCLASS()
class UENET_API AGASLesson1Character : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	/** 构造函数：创建 ASC、AttributeSet、相机和调试显示组件。 */
	AGASLesson1Character();

	/** 初始化 GAS ActorInfo，绑定属性变化监听。 */
	virtual void BeginPlay() override;

	/** 本地移动和测试按键。 */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** 返回本角色持有的 AbilitySystemComponent。 */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** 读取当前生命值。 */
	UFUNCTION(BlueprintPure, Category = "GAS Lesson 1")
	float GetCurrentHealth() const;

	/** 读取当前法力值。 */
	UFUNCTION(BlueprintPure, Category = "GAS Lesson 1")
	float GetCurrentMana() const;

protected:
	/** GAS 核心组件，负责属性修改、监听和后续 Ability/Effect 扩展。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS Lesson 1")
	UAbilitySystemComponent* AbilitySystemComponent;

	/** GASLesson1 属性集合。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS Lesson 1")
	UGASLesson1AttributeSet* AttributeSet;

	/** 第一人称观察相机。 */
	UPROPERTY(VisibleAnywhere, Category = "GAS Lesson 1")
	UCameraComponent* Camera;

	/** 用于表现角色位置的演示 Mesh。 */
	UPROPERTY(VisibleAnywhere, Category = "GAS Lesson 1")
	UStaticMeshComponent* DemoMesh;

	/** 显示属性状态的头顶文字。 */
	UPROPERTY(VisibleAnywhere, Category = "GAS Lesson 1")
	UTextRenderComponent* AttributeText;

	/** 默认最大生命值。 */
	UPROPERTY(EditAnywhere, Category = "GAS Lesson 1|Default Attribute")
	float DefaultMaxHealth = 100.f;

	/** 默认生命值。 */
	UPROPERTY(EditAnywhere, Category = "GAS Lesson 1|Default Attribute")
	float DefaultHealth = 100.f;

	/** 默认最大法力值。 */
	UPROPERTY(EditAnywhere, Category = "GAS Lesson 1|Default Attribute")
	float DefaultMaxMana = 100.f;

	/** 默认法力值。 */
	UPROPERTY(EditAnywhere, Category = "GAS Lesson 1|Default Attribute")
	float DefaultMana = 100.f;

	/** 默认攻击力。 */
	UPROPERTY(EditAnywhere, Category = "GAS Lesson 1|Default Attribute")
	float DefaultAttackPower = 20.f;

	/** 默认防御力。 */
	UPROPERTY(EditAnywhere, Category = "GAS Lesson 1|Default Attribute")
	float DefaultDefense = 10.f;

	/** F 键测试伤害。 */
	UPROPERTY(EditAnywhere, Category = "GAS Lesson 1|Debug")
	float DebugDamageAmount = 10.f;

	/** W 键向前移动。 */
	void Input_MoveForward(float Value);

	/** S 键向后移动。 */
	void Input_MoveBackward(float Value);

	/** D 键向右移动。 */
	void Input_MoveRight(float Value);

	/** A 键向左移动。 */
	void Input_MoveLeft(float Value);

	/** 鼠标 X 轴转向。 */
	void Input_Turn(float Value);

	/** 鼠标 Y 轴抬头 / 低头。 */
	void Input_LookUp(float Value);

	/** F 键：请求服务器扣血。 */
	void Input_DebugDamage();

	/** 服务器执行属性修改。 */
	UFUNCTION(Server, Reliable)
	void ServerDebugDamage();

private:
	/** 初始化 ASC 的 OwnerActor 和 AvatarActor。 */
	void InitializeAbilityActorInfo();

	/** 服务器初始化默认属性值。 */
	void InitializeDefaultAttributes();

	/** 绑定属性变化事件。 */
	void BindAttributeChangeDelegates();

	/** Health 变化回调。 */
	void HandleHealthChanged(const FOnAttributeChangeData& Data);

	/** Mana 变化回调。 */
	void HandleManaChanged(const FOnAttributeChangeData& Data);

	/** MaxHealth 变化回调。 */
	void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);

	/** MaxMana 变化回调。 */
	void HandleMaxManaChanged(const FOnAttributeChangeData& Data);

	/** 刷新头顶属性文字。 */
	void UpdateAttributeText();

	/** 输出属性变化调试信息。 */
	void ShowAttributeMessage(const FString& Message, const FColor& Color = FColor::Cyan) const;
};
