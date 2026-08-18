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
	constexpr int32 AttackMultiplyIndex = 10;
	constexpr int32 LevelDamageIndex = 11;
	constexpr int32 ApplyOiledIndex = 12;
	constexpr int32 BurningRequiresOilIndex = 13;
	constexpr int32 DamageAimedTargetIndex = 14;
	constexpr int32 AttributeBasedHealIndex = 15;
	constexpr int32 MMCHealIndex = 16;
	constexpr int32 ExecutionDamageAimedTargetIndex = 17;
	constexpr int32 CleanseIndex = 18;
	constexpr int32 DamageImmunityIndex = 19;
	constexpr int32 MaxTestIndex = 19;

	const FName DynamicDamageTagName(TEXT("Data.Damage"));
	const FName DemoEffectTagName(TEXT("Demo.Effect"));
	const FName OiledTagName(TEXT("State.Oiled"));
	const FName BaseDamageTagName(TEXT("Data.BaseDamage"));
	const FName DamageImmuneTagName(TEXT("State.DamageImmune"));
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
	// E/F5 的服务器射线使用 Visibility；只开放这一项，避免教学角色参与其他碰撞。
	DemoMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

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
	PlayerInputComponent->BindKey(EKeys::F1, IE_Pressed, this, &ThisClass::TestAttackMultiply);
	PlayerInputComponent->BindKey(EKeys::F2, IE_Pressed, this, &ThisClass::TestLevelDamage);
	PlayerInputComponent->BindKey(EKeys::F3, IE_Pressed, this, &ThisClass::TestApplyOiled);
	PlayerInputComponent->BindKey(EKeys::F4, IE_Pressed, this, &ThisClass::TestBurningRequiresOil);
	PlayerInputComponent->BindKey(EKeys::F5, IE_Pressed, this, &ThisClass::TestDamageAimedTarget);
	PlayerInputComponent->BindKey(EKeys::Q, IE_Pressed, this, &ThisClass::TestAttributeBasedHeal);
	PlayerInputComponent->BindKey(EKeys::W, IE_Pressed, this, &ThisClass::TestMMCHeal);
	PlayerInputComponent->BindKey(EKeys::E, IE_Pressed, this, &ThisClass::TestExecutionDamageAimedTarget);
	PlayerInputComponent->BindKey(EKeys::R, IE_Pressed, this, &ThisClass::TestCleanse);
	PlayerInputComponent->BindKey(EKeys::T, IE_Pressed, this, &ThisClass::TestDamageImmunity);
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
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UGASLesson2AttributeSet::GetDefenseAttribute()).AddUObject(this, &ThisClass::OnDefenseChanged);
}

FActiveGameplayEffectHandle AGASLesson2Character::ApplyEffectToSelf(
	TSubclassOf<UGameplayEffect> EffectClass,
	float EffectLevel)
{
	// 第一步：ASC 负责创建和应用 GE，EffectClass 则提供 GE 的配置模板。
	// 任何一个不存在，都无法继续创建本次 GameplayEffectSpec。
	if (!AbilitySystemComponent || !EffectClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[GAS] ASC or EffectClass is not configured."));
		return {};
	}

	// 第二步：创建本次效果的 Context，也就是“这次效果从哪里来”。
	// MakeEffectContext 会根据 ASC 的 ActorInfo 自动记录 OwnerActor 和 AvatarActor。
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();

	// SourceObject 是与本次效果关联的来源对象。
	// 本课没有单独的武器或技能对象，所以使用 Character 自己。
	EffectContext.AddSourceObject(this);

	// 第三步：由 GE 模板、GE 等级和 Context 创建本次执行使用的 Spec。
	// EffectLevel 可被 ScalableFloat、MMC 等数值计算使用。
	FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
		EffectClass,
		EffectLevel,
		EffectContext);
	if (!EffectSpecHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[GAS] Failed to create GameplayEffectSpec: %s"), *GetNameSafe(EffectClass.Get()));
		return {};
	}

	UE_LOG(LogTemp, Warning, TEXT("[GAS] Create GameplayEffectSpec: %s, Level: %.1f"),
		*GetNameSafe(EffectClass.Get()), EffectLevel);
	UE_LOG(LogTemp, Warning, TEXT("[GAS] Apply GameplayEffectSpec"));

	// 第四步：Handle 中的 Data 才是真正的 FGameplayEffectSpec。
	// ApplyGameplayEffectSpecToSelf 会把这份 Spec 应用给当前 ASC 自己。
	// Duration/Infinite GE 可返回用于查询或移除的 ActiveGameplayEffectHandle；
	// Instant GE 执行后立即结束，不会作为持续效果保存在 ASC 中。
	const FGameplayEffectSpec& EffectSpec = *EffectSpecHandle.Data.Get();
	return AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(EffectSpec);
}

FActiveGameplayEffectHandle AGASLesson2Character::ApplyEffectToTarget(
	TSubclassOf<UGameplayEffect> EffectClass,
	UAbilitySystemComponent* TargetAbilitySystem,
	float EffectLevel)
{
	// AbilitySystemComponent 是来源 ASC，负责创建这次效果；
	// TargetAbilitySystem 是目标 ASC，负责接收这次效果。
	// GE 模板、来源 ASC 或目标 ASC 缺少任何一个，都不能完成应用。
	if (!AbilitySystemComponent || !TargetAbilitySystem || !EffectClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[GAS] Target Effect, source ASC, or target ASC is not configured."));
		return {};
	}

	// 第一步：由来源 ASC 创建 Context。
	// Context 记录“是谁发起了这次效果”，而不是记录效果要应用给谁。
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();

	// 本课没有单独的武器或技能对象，因此把来源 Character 记录为 SourceObject。
	EffectContext.AddSourceObject(this);

	// 第二步：仍然由来源 ASC 创建 Spec。
	// EffectLevel、Context 以及后续的 SetByCaller 数据都属于这一次 Spec。
	FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
		EffectClass,
		EffectLevel,
		EffectContext);
	if (!EffectSpecHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[GAS] Failed to create target GameplayEffectSpec: %s"), *GetNameSafe(EffectClass.Get()));
		return {};
	}

	UE_LOG(LogTemp, Warning, TEXT("[GAS] Apply %s (Level %.1f) from %s to %s"),
		*GetNameSafe(EffectClass.Get()), EffectLevel, *GetName(), *GetNameSafe(TargetAbilitySystem->GetAvatarActor()));

	// 第三步：从 Handle 中取得真正的 Spec，并通过来源 ASC 应用给目标 ASC。
	// 与 ApplyGameplayEffectSpecToSelf 的核心区别只有目标：
	// ToSelf 使用自己的 ASC，ToTarget 使用传入的 TargetAbilitySystem。
	const FGameplayEffectSpec& EffectSpec = *EffectSpecHandle.Data.Get();
	return AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(EffectSpec, TargetAbilitySystem);
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

bool AGASLesson2Character::FindAimedTarget(
	FHitResult& OutHitResult,
	UAbilitySystemComponent*& OutTargetAbilitySystem) const
{
	OutHitResult = {};
	OutTargetAbilitySystem = nullptr;

	if (!HasAuthority() || !GetWorld())
	{
		return false;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	GetActorEyesViewPoint(ViewLocation, ViewRotation);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(GASLesson2TargetTrace), true, this);
	const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * TargetEffectRange;
	if (!GetWorld()->LineTraceSingleByChannel(OutHitResult, ViewLocation, TraceEnd, ECC_Visibility, QueryParams))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GAS] No aimed target found within %.0f units."), TargetEffectRange);
		return false;
	}

	AActor* TargetActor = OutHitResult.GetActor();
	if (!TargetActor || TargetActor == this || FVector::DistSquared(GetActorLocation(), TargetActor->GetActorLocation()) > FMath::Square(TargetEffectRange))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GAS] Aimed target is invalid or out of range."));
		return false;
	}

	IAbilitySystemInterface* TargetAbilitySystemInterface = Cast<IAbilitySystemInterface>(TargetActor);
	OutTargetAbilitySystem = TargetAbilitySystemInterface
		? TargetAbilitySystemInterface->GetAbilitySystemComponent()
		: nullptr;
	if (!OutTargetAbilitySystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GAS] Aimed target %s has no AbilitySystemComponent."), *GetNameSafe(TargetActor));
		return false;
	}

	return true;
}

void AGASLesson2Character::ApplyDamageToAimedTarget()
{
	FHitResult HitResult;
	UAbilitySystemComponent* TargetAbilitySystem = nullptr;
	if (FindAimedTarget(HitResult, TargetAbilitySystem))
	{
		ApplyEffectToTarget(DamageEffect, TargetAbilitySystem);
	}
}

void AGASLesson2Character::ApplyExecutionDamageToAimedTarget()
{
	if (!AbilitySystemComponent || !ExecutionDamageEffect)
	{
		UE_LOG(LogTemp, Error, TEXT("[GAS] ExecutionDamageEffect is not configured."));
		return;
	}

	FHitResult HitResult;
	UAbilitySystemComponent* TargetAbilitySystem = nullptr;
	if (!FindAimedTarget(HitResult, TargetAbilitySystem))
	{
		return;
	}

	const FGameplayTag BaseDamageTag = FGameplayTag::RequestGameplayTag(GASLesson2::BaseDamageTagName, false);
	if (!BaseDamageTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[GAS] GameplayTag Data.BaseDamage is not registered."));
		return;
	}

	// Context 保存来源与命中信息；Execution 或 GameplayCue 可以从 Spec 中取回它们。
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	EffectContext.AddHitResult(HitResult);

	FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
		ExecutionDamageEffect,
		1.f,
		EffectContext);
	if (!EffectSpecHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[GAS] Failed to create Execution damage Spec."));
		return;
	}

	// BaseDamage 是这次攻击的动态输入；Attack 和 Defense 则由 Execution 自己捕获。
	EffectSpecHandle.Data->SetSetByCallerMagnitude(BaseDamageTag, FMath::Max(ExecutionBaseDamage, 0.f));
	AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(), TargetAbilitySystem);
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
	UE_LOG(LogTemp, Warning, TEXT("Defense    BaseValue: %.2f  CurrentValue: %.2f"), AttributeSet->Defense.GetBaseValue(), AttributeSet->Defense.GetCurrentValue());
	UE_LOG(LogTemp, Warning, TEXT("===================================="));

	ShowDebugMessage(FString::Printf(TEXT("Health %.0f / %.0f | Attack %.0f | Defense %.0f"), AttributeSet->GetHealth(), AttributeSet->GetMaxHealth(), AttributeSet->GetAttack(), AttributeSet->GetDefense()), FColor::Green);
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

void AGASLesson2Character::OnDefenseChanged(const FOnAttributeChangeData& Data)
{
	UE_LOG(LogTemp, Warning, TEXT("[GAS] Defense: %.2f -> %.2f"), Data.OldValue, Data.NewValue);
	ShowDebugMessage(FString::Printf(TEXT("Defense: %.0f -> %.0f"), Data.OldValue, Data.NewValue), FColor::Silver);
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
	case GASLesson2::AttackMultiplyIndex:
		ApplyEffectToSelf(AttackMultiplyEffect);
		break;
	case GASLesson2::LevelDamageIndex:
		{
			const int32 AppliedLevel = NextLevelDamageTestLevel;
			ApplyEffectToSelf(LevelDamageEffect, static_cast<float>(AppliedLevel));
			NextLevelDamageTestLevel = AppliedLevel >= 3 ? 1 : AppliedLevel + 1;
			UE_LOG(LogTemp, Warning, TEXT("[GAS] Applied level-scaled damage at GE Level %d. Next level: %d."), AppliedLevel, NextLevelDamageTestLevel);
		}
		break;
	case GASLesson2::ApplyOiledIndex:
		{
			const FGameplayTag OiledTag = FGameplayTag::RequestGameplayTag(GASLesson2::OiledTagName, false);
			if (OiledTag.IsValid() && AbilitySystemComponent->HasMatchingGameplayTag(OiledTag))
			{
				UE_LOG(LogTemp, Warning, TEXT("[GAS] State.Oiled is already active."));
			}
			else
			{
				ApplyEffectToSelf(OiledEffect);
			}
		}
		break;
	case GASLesson2::BurningRequiresOilIndex:
		if (!ApplyEffectToSelf(BurningRequiresOilEffect).IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("[GAS] Burning was rejected. Apply State.Oiled with F3 first."));
		}
		break;
	case GASLesson2::DamageAimedTargetIndex:
		ApplyDamageToAimedTarget();
		break;
	case GASLesson2::AttributeBasedHealIndex:
		ApplyEffectToSelf(AttributeBasedHealEffect);
		break;
	case GASLesson2::MMCHealIndex:
		ApplyEffectToSelf(MMCHealEffect);
		break;
	case GASLesson2::ExecutionDamageAimedTargetIndex:
		ApplyExecutionDamageToAimedTarget();
		break;
	case GASLesson2::CleanseIndex:
		ApplyEffectToSelf(CleanseEffect);
		break;
	case GASLesson2::DamageImmunityIndex:
		{
			const FGameplayTag DamageImmuneTag = FGameplayTag::RequestGameplayTag(GASLesson2::DamageImmuneTagName, false);
			if (DamageImmuneTag.IsValid() && AbilitySystemComponent->HasMatchingGameplayTag(DamageImmuneTag))
			{
				UE_LOG(LogTemp, Warning, TEXT("[GAS] Damage immunity is already active. Press 0 to clear it."));
			}
			else
			{
				ApplyEffectToSelf(DamageImmunityEffect);
			}
		}
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
void AGASLesson2Character::TestAttackMultiply() { RequestTest(GASLesson2::AttackMultiplyIndex); }
void AGASLesson2Character::TestLevelDamage() { RequestTest(GASLesson2::LevelDamageIndex); }
void AGASLesson2Character::TestApplyOiled() { RequestTest(GASLesson2::ApplyOiledIndex); }
void AGASLesson2Character::TestBurningRequiresOil() { RequestTest(GASLesson2::BurningRequiresOilIndex); }
void AGASLesson2Character::TestDamageAimedTarget() { RequestTest(GASLesson2::DamageAimedTargetIndex); }
void AGASLesson2Character::TestAttributeBasedHeal() { RequestTest(GASLesson2::AttributeBasedHealIndex); }
void AGASLesson2Character::TestMMCHeal() { RequestTest(GASLesson2::MMCHealIndex); }
void AGASLesson2Character::TestExecutionDamageAimedTarget() { RequestTest(GASLesson2::ExecutionDamageAimedTargetIndex); }
void AGASLesson2Character::TestCleanse() { RequestTest(GASLesson2::CleanseIndex); }
void AGASLesson2Character::TestDamageImmunity() { RequestTest(GASLesson2::DamageImmunityIndex); }

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
