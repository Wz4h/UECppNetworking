// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/GASLesson2/GASLesson2Character.h"

#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "InputCoreTypes.h"
#include "NetLessons/GASLesson2/GASLesson2AttributeSet.h"
#include "UObject/ConstructorHelpers.h"

namespace GASLesson2
{
	constexpr int32 ClearEffectsIndex = 0;
	constexpr int32 DamageIndex = 1;
	constexpr int32 HealIndex = 2;
	constexpr int32 AttackBuffIndex = 3;
	constexpr int32 EquipWeaponIndex = 4;
	constexpr int32 RemoveWeaponIndex = 5;
	constexpr int32 DynamicDamageIndex = 6;
	constexpr int32 PoisonIndex = 7;
	constexpr int32 StackAttackIndex = 8;
	constexpr int32 PrintAttributesIndex = 9;
	constexpr int32 MaxTestIndex = 9;

	const FName DynamicDamageTagName(TEXT("Data.Damage"));
	const FName DemoEffectTagName(TEXT("Demo.Effect"));
}

AGASLesson2Character::AGASLesson2Character()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	// ASC 直接位于 Character 上，Full 模式最适合观察本课的 Active GameplayEffect。
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Full);

	// ASC 初始化时会扫描同一 Owner 下的默认 AttributeSet 子对象并自动注册。
	AttributeSet = CreateDefaultSubobject<UGASLesson2AttributeSet>(TEXT("AttributeSet"));

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(GetCapsuleComponent());
	Camera->SetRelativeLocation(FVector(0.f, 0.f, 70.f));
	Camera->bUsePawnControlRotation = true;

	DemoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DemoMesh"));
	DemoMesh->SetupAttachment(GetCapsuleComponent());
	DemoMesh->SetRelativeLocation(FVector(0.f, 0.f, -20.f));
	DemoMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 1.8f));
	DemoMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DemoMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		DemoMesh->SetStaticMesh(CubeMesh.Object);
	}
}

void AGASLesson2Character::BeginPlay()
{
	Super::BeginPlay();

	InitializeAbilitySystem();
	BindAttributeDelegates();

	if (HasAuthority())
	{
		PrintAttributes();
	}
}

void AGASLesson2Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindKey(EKeys::Zero, IE_Pressed, this, &ThisClass::TestClearEffects);
	PlayerInputComponent->BindKey(EKeys::One, IE_Pressed, this, &ThisClass::TestDamage);
	PlayerInputComponent->BindKey(EKeys::Two, IE_Pressed, this, &ThisClass::TestHeal);
	PlayerInputComponent->BindKey(EKeys::Three, IE_Pressed, this, &ThisClass::TestAttackBuff);
	PlayerInputComponent->BindKey(EKeys::Four, IE_Pressed, this, &ThisClass::TestEquipWeapon);
	PlayerInputComponent->BindKey(EKeys::Five, IE_Pressed, this, &ThisClass::TestRemoveWeapon);
	PlayerInputComponent->BindKey(EKeys::Six, IE_Pressed, this, &ThisClass::TestDynamicDamage);
	PlayerInputComponent->BindKey(EKeys::Seven, IE_Pressed, this, &ThisClass::TestPoison);
	PlayerInputComponent->BindKey(EKeys::Eight, IE_Pressed, this, &ThisClass::TestStackAttack);
	PlayerInputComponent->BindKey(EKeys::Nine, IE_Pressed, this, &ThisClass::TestPrintAttributes);
}

UAbilitySystemComponent* AGASLesson2Character::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AGASLesson2Character::InitializeAbilitySystem()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

void AGASLesson2Character::BindAttributeDelegates()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UGASLesson2AttributeSet::GetHealthAttribute()).AddUObject(this, &ThisClass::OnHealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UGASLesson2AttributeSet::GetAttackAttribute()).AddUObject(this, &ThisClass::OnAttackChanged);
}

FActiveGameplayEffectHandle AGASLesson2Character::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level)
{
	if (!AbilitySystemComponent || !EffectClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[GAS] EffectClass is not configured."));
		return {};
	}

	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, Level, Context);
	if (!SpecHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[GAS] Failed to create GameplayEffectSpec: %s"), *GetNameSafe(EffectClass.Get()));
		return {};
	}

	UE_LOG(LogTemp, Warning, TEXT("[GAS] Create GameplayEffectSpec: %s"), *GetNameSafe(EffectClass.Get()));
	UE_LOG(LogTemp, Warning, TEXT("[GAS] Apply GameplayEffectSpec"));
	return AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}

void AGASLesson2Character::ApplyDynamicDamage(float Damage)
{
	if (!AbilitySystemComponent || !DynamicDamageEffect)
	{
		UE_LOG(LogTemp, Error, TEXT("[GAS] DynamicDamageEffect is not configured."));
		return;
	}

	const FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(GASLesson2::DynamicDamageTagName, false);
	if (!DamageTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[GAS] GameplayTag Data.Damage is not registered."));
		return;
	}

	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	Context.AddSourceObject(this);
	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DynamicDamageEffect, 1.f, Context);
	if (!SpecHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[GAS] Failed to create GameplayEffectSpec: %s"), *GetNameSafe(DynamicDamageEffect.Get()));
		return;
	}

	const float DamageMagnitude = -FMath::Abs(Damage);
	SpecHandle.Data->SetSetByCallerMagnitude(DamageTag, DamageMagnitude);

	UE_LOG(LogTemp, Warning, TEXT("[GAS] Create GameplayEffectSpec: %s"), *GetNameSafe(DynamicDamageEffect.Get()));
	UE_LOG(LogTemp, Warning, TEXT("[GAS] SetByCaller Data.Damage = %.2f"), DamageMagnitude);
	UE_LOG(LogTemp, Warning, TEXT("[GAS] Apply GameplayEffectSpec"));
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}

void AGASLesson2Character::RemoveWeaponEffect()
{
	if (!AbilitySystemComponent || !WeaponEffectHandle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GAS] Weapon Effect is not active."));
		return;
	}

	const bool bRemoved = AbilitySystemComponent->RemoveActiveGameplayEffect(WeaponEffectHandle);
	if (bRemoved)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GAS] Remove Weapon Effect"));
		WeaponEffectHandle.Invalidate();
	}
}

void AGASLesson2Character::ClearDemoEffects()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	const FGameplayTag DemoEffectTag = FGameplayTag::RequestGameplayTag(GASLesson2::DemoEffectTagName, false);
	if (!DemoEffectTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[GAS] GameplayTag Demo.Effect is not registered."));
		return;
	}

	FGameplayTagContainer DemoEffectTags;
	DemoEffectTags.AddTag(DemoEffectTag);
	const FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(DemoEffectTags);
	const int32 RemovedCount = AbilitySystemComponent->RemoveActiveEffects(Query);
	WeaponEffectHandle.Invalidate();

	UE_LOG(LogTemp, Warning, TEXT("[GAS] Clear Demo Effects: removed %d active effect(s)."), RemovedCount);
}

void AGASLesson2Character::PrintAttributes() const
{
	if (!AttributeSet)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("========== GAS ATTRIBUTES =========="));
	UE_LOG(LogTemp, Warning, TEXT("Health     BaseValue: %.2f  CurrentValue: %.2f"), AttributeSet->Health.GetBaseValue(), AttributeSet->Health.GetCurrentValue());
	UE_LOG(LogTemp, Warning, TEXT("MaxHealth  BaseValue: %.2f  CurrentValue: %.2f"), AttributeSet->MaxHealth.GetBaseValue(), AttributeSet->MaxHealth.GetCurrentValue());
	UE_LOG(LogTemp, Warning, TEXT("Attack     BaseValue: %.2f  CurrentValue: %.2f"), AttributeSet->Attack.GetBaseValue(), AttributeSet->Attack.GetCurrentValue());
	UE_LOG(LogTemp, Warning, TEXT("===================================="));

	ShowDebugMessage(FString::Printf(TEXT("Health %.0f / %.0f | Attack %.0f"), AttributeSet->GetHealth(), AttributeSet->GetMaxHealth(), AttributeSet->GetAttack()), FColor::Green);
}

void AGASLesson2Character::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	UE_LOG(LogTemp, Warning, TEXT("[GAS] Health: %.2f -> %.2f"), Data.OldValue, Data.NewValue);
	ShowDebugMessage(FString::Printf(TEXT("Health: %.0f -> %.0f"), Data.OldValue, Data.NewValue));
}

void AGASLesson2Character::OnAttackChanged(const FOnAttributeChangeData& Data)
{
	UE_LOG(LogTemp, Warning, TEXT("[GAS] Attack: %.2f -> %.2f"), Data.OldValue, Data.NewValue);
	ShowDebugMessage(FString::Printf(TEXT("Attack: %.0f -> %.0f"), Data.OldValue, Data.NewValue), FColor::Yellow);
}

void AGASLesson2Character::RequestTest(int32 TestIndex)
{
	if (TestIndex < GASLesson2::ClearEffectsIndex || TestIndex > GASLesson2::MaxTestIndex)
	{
		return;
	}

	if (HasAuthority())
	{
		ExecuteTest(TestIndex);
	}
	else
	{
		ServerApplyTestEffect(TestIndex);
	}
}

void AGASLesson2Character::ExecuteTest(int32 TestIndex)
{
	if (!HasAuthority())
	{
		return;
	}

	switch (TestIndex)
	{
	case GASLesson2::ClearEffectsIndex:
		ClearDemoEffects();
		break;
	case GASLesson2::DamageIndex:
		ApplyEffectToSelf(DamageEffect);
		break;
	case GASLesson2::HealIndex:
		ApplyEffectToSelf(HealEffect);
		break;
	case GASLesson2::AttackBuffIndex:
		ApplyEffectToSelf(AttackBuffEffect);
		break;
	case GASLesson2::EquipWeaponIndex:
		if (WeaponEffectHandle.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("[GAS] Weapon Effect already active."));
		}
		else
		{
			WeaponEffectHandle = ApplyEffectToSelf(WeaponEffect);
		}
		break;
	case GASLesson2::RemoveWeaponIndex:
		RemoveWeaponEffect();
		break;
	case GASLesson2::DynamicDamageIndex:
		ApplyDynamicDamage(DynamicDamageAmount);
		break;
	case GASLesson2::PoisonIndex:
		ApplyEffectToSelf(PoisonEffect);
		break;
	case GASLesson2::StackAttackIndex:
		{
			const FActiveGameplayEffectHandle Handle = ApplyEffectToSelf(StackAttackEffect);
			if (Handle.IsValid())
			{
				const int32 StackCount = AbilitySystemComponent->GetCurrentStackCount(Handle);
				UE_LOG(LogTemp, Warning, TEXT("[GAS] Attack Buff Stack: %d / 5"), StackCount);
			}
		}
		break;
	case GASLesson2::PrintAttributesIndex:
		PrintAttributes();
		break;
	default:
		break;
	}
}

void AGASLesson2Character::TestDamage() { RequestTest(GASLesson2::DamageIndex); }
void AGASLesson2Character::TestHeal() { RequestTest(GASLesson2::HealIndex); }
void AGASLesson2Character::TestAttackBuff() { RequestTest(GASLesson2::AttackBuffIndex); }
void AGASLesson2Character::TestEquipWeapon() { RequestTest(GASLesson2::EquipWeaponIndex); }
void AGASLesson2Character::TestRemoveWeapon() { RequestTest(GASLesson2::RemoveWeaponIndex); }
void AGASLesson2Character::TestDynamicDamage() { RequestTest(GASLesson2::DynamicDamageIndex); }
void AGASLesson2Character::TestPoison() { RequestTest(GASLesson2::PoisonIndex); }
void AGASLesson2Character::TestStackAttack() { RequestTest(GASLesson2::StackAttackIndex); }
void AGASLesson2Character::TestPrintAttributes()
{
	// 客户端先打印自己当前收到的复制值，再让服务器打印权威值，便于多人对照。
	PrintAttributes();
	if (!HasAuthority())
	{
		ServerApplyTestEffect(GASLesson2::PrintAttributesIndex);
	}
}
void AGASLesson2Character::TestClearEffects() { RequestTest(GASLesson2::ClearEffectsIndex); }

void AGASLesson2Character::ServerApplyTestEffect_Implementation(int32 TestIndex)
{
	ExecuteTest(TestIndex);
}

void AGASLesson2Character::ShowDebugMessage(const FString& Message, const FColor& Color) const
{
	if (GEngine && IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.5f, Color, FString::Printf(TEXT("[GAS] %s"), *Message));
	}
}
