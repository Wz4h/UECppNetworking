// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/GASLesson1/GASLesson1Character.h"

#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputCoreTypes.h"
#include "NetLessons/GASLesson1/GASLesson1AttributeSet.h"
#include "UObject/ConstructorHelpers.h"

#define LOCTEXT_NAMESPACE "GASLesson1Character"

AGASLesson1Character::AGASLesson1Character()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = 450.f;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	// ASC 必须开启复制，服务器上发生的属性变化才有机会同步到客户端。
	AbilitySystemComponent->SetIsReplicated(true);
	// Full 模式会复制完整的 GameplayEffect 信息；本课还没讲 GameplayEffect，
	// 这里先用最直观的模式，便于后续课程继续扩展。
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Full);

	// AttributeSet 作为角色的默认子对象创建后，会被 ASC 发现并管理。
	// 它只保存属性，不负责输入、UI 或具体扣血规则。
	AttributeSet = CreateDefaultSubobject<UGASLesson1AttributeSet>(TEXT("AttributeSet"));

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
	DemoMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	AttributeText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("AttributeText"));
	AttributeText->SetupAttachment(GetCapsuleComponent());
	AttributeText->SetRelativeLocation(FVector(0.f, 0.f, 140.f));
	AttributeText->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	AttributeText->SetHorizontalAlignment(EHTA_Center);
	AttributeText->SetWorldSize(18.f);
	AttributeText->SetTextRenderColor(FColor::White);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		DemoMesh->SetStaticMesh(CubeMesh.Object);
	}
}

void AGASLesson1Character::BeginPlay()
{
	Super::BeginPlay();

	InitializeAbilityActorInfo();

	if (HasAuthority())
	{
		InitializeDefaultAttributes();
	}

	BindAttributeChangeDelegates();
	UpdateAttributeText();
}

void AGASLesson1Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindKey(EKeys::F, IE_Pressed, this, &ThisClass::Input_DebugDamage);
	PlayerInputComponent->BindAxisKey(EKeys::W, this, &ThisClass::Input_MoveForward);
	PlayerInputComponent->BindAxisKey(EKeys::S, this, &ThisClass::Input_MoveBackward);
	PlayerInputComponent->BindAxisKey(EKeys::D, this, &ThisClass::Input_MoveRight);
	PlayerInputComponent->BindAxisKey(EKeys::A, this, &ThisClass::Input_MoveLeft);
	PlayerInputComponent->BindAxisKey(EKeys::MouseX, this, &ThisClass::Input_Turn);
	PlayerInputComponent->BindAxisKey(EKeys::MouseY, this, &ThisClass::Input_LookUp);
}

UAbilitySystemComponent* AGASLesson1Character::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

float AGASLesson1Character::GetCurrentHealth() const
{
	return AbilitySystemComponent ? AbilitySystemComponent->GetNumericAttribute(UGASLesson1AttributeSet::GetHealthAttribute()) : 0.f;
}

float AGASLesson1Character::GetCurrentMana() const
{
	return AbilitySystemComponent ? AbilitySystemComponent->GetNumericAttribute(UGASLesson1AttributeSet::GetManaAttribute()) : 0.f;
}

void AGASLesson1Character::Input_MoveForward(float Value)
{
	if (FMath::IsNearlyZero(Value))
	{
		return;
	}

	const FRotator ControlRotation = Controller ? Controller->GetControlRotation() : GetActorRotation();
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Value);
}

void AGASLesson1Character::Input_MoveBackward(float Value)
{
	if (FMath::IsNearlyZero(Value))
	{
		return;
	}

	const FRotator ControlRotation = Controller ? Controller->GetControlRotation() : GetActorRotation();
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), -Value);
}

void AGASLesson1Character::Input_MoveRight(float Value)
{
	if (FMath::IsNearlyZero(Value))
	{
		return;
	}

	const FRotator ControlRotation = Controller ? Controller->GetControlRotation() : GetActorRotation();
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Value);
}

void AGASLesson1Character::Input_MoveLeft(float Value)
{
	if (FMath::IsNearlyZero(Value))
	{
		return;
	}

	const FRotator ControlRotation = Controller ? Controller->GetControlRotation() : GetActorRotation();
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), -Value);
}

void AGASLesson1Character::Input_Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AGASLesson1Character::Input_LookUp(float Value)
{
	AddControllerPitchInput(-Value);
}

void AGASLesson1Character::Input_DebugDamage()
{
	ShowAttributeMessage(TEXT("Client Request: Health -10"), FColor::Yellow);
	// 客户端不能直接改 Health，只能请求服务器。
	// 真正的数值修改发生在 ServerDebugDamage_Implementation。
	ServerDebugDamage();
}

void AGASLesson1Character::ServerDebugDamage_Implementation()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	const float OldHealth = GetCurrentHealth();
	// 不直接写 AttributeSet->SetHealth(Health - 10)，而是通过 ASC 修改属性。
	// 这样这节课先演示 GAS 的属性修改入口，下一节接 GameplayEffect 时流程可以自然升级。
	AbilitySystemComponent->ApplyModToAttribute(UGASLesson1AttributeSet::GetHealthAttribute(), EGameplayModOp::Additive, -DebugDamageAmount);
	const float NewHealth = GetCurrentHealth();

	const FString Message = FString::Printf(TEXT("Server Damage: Health %.0f -> %.0f"), OldHealth, NewHealth);
	ShowAttributeMessage(Message, FColor::Green);
	UpdateAttributeText();
}

void AGASLesson1Character::InitializeAbilityActorInfo()
{
	if (AbilitySystemComponent)
	{
		// InitAbilityActorInfo 告诉 ASC：
		// - OwnerActor：谁拥有这套能力系统。
		// - AvatarActor：能力实际作用在哪个 Actor 身上。
		// 本课 ASC 直接放在 Character 上，所以两个参数都传 this。
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

void AGASLesson1Character::InitializeDefaultAttributes()
{
	if (!AttributeSet)
	{
		return;
	}

	AttributeSet->InitMaxHealth(DefaultMaxHealth);
	AttributeSet->InitHealth(FMath::Clamp(DefaultHealth, 0.f, DefaultMaxHealth));
	AttributeSet->InitMaxMana(DefaultMaxMana);
	AttributeSet->InitMana(FMath::Clamp(DefaultMana, 0.f, DefaultMaxMana));
	AttributeSet->InitAttackPower(DefaultAttackPower);
	AttributeSet->InitDefense(DefaultDefense);
}

void AGASLesson1Character::BindAttributeChangeDelegates()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// 监听属性变化不需要 Tick 轮询。
	// 当服务器修改属性，或者客户端收到复制并经过 GAMEPLAYATTRIBUTE_REPNOTIFY 后，
	// ASC 会广播对应属性的 Delegate，然后这里绑定的函数刷新显示。
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UGASLesson1AttributeSet::GetHealthAttribute()).AddUObject(this, &ThisClass::HandleHealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UGASLesson1AttributeSet::GetManaAttribute()).AddUObject(this, &ThisClass::HandleManaChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UGASLesson1AttributeSet::GetMaxHealthAttribute()).AddUObject(this, &ThisClass::HandleMaxHealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UGASLesson1AttributeSet::GetMaxManaAttribute()).AddUObject(this, &ThisClass::HandleMaxManaChanged);
}

void AGASLesson1Character::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	const FString Message = FString::Printf(TEXT("Health Changed: %.0f -> %.0f"), Data.OldValue, Data.NewValue);
	ShowAttributeMessage(Message);
	UpdateAttributeText();
}

void AGASLesson1Character::HandleManaChanged(const FOnAttributeChangeData& Data)
{
	const FString Message = FString::Printf(TEXT("Mana Changed: %.0f -> %.0f"), Data.OldValue, Data.NewValue);
	ShowAttributeMessage(Message);
	UpdateAttributeText();
}

void AGASLesson1Character::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	const FString Message = FString::Printf(TEXT("MaxHealth Changed: %.0f -> %.0f"), Data.OldValue, Data.NewValue);
	ShowAttributeMessage(Message);
	UpdateAttributeText();
}

void AGASLesson1Character::HandleMaxManaChanged(const FOnAttributeChangeData& Data)
{
	const FString Message = FString::Printf(TEXT("MaxMana Changed: %.0f -> %.0f"), Data.OldValue, Data.NewValue);
	ShowAttributeMessage(Message);
	UpdateAttributeText();
}

void AGASLesson1Character::UpdateAttributeText()
{
	if (!AttributeText || !AttributeSet)
	{
		return;
	}

	const FText Text = FText::Format(
		LOCTEXT(
			"AttributeText",
			"GASLesson1 属性集\n生命值: {0} / {1}\n法力值: {2} / {3}\n攻击: {4} 防御: {5}\nF: 服务器扣除 10 点生命值"
		),
		FText::AsNumber(FMath::RoundToInt(AttributeSet->GetHealth())),
		FText::AsNumber(FMath::RoundToInt(AttributeSet->GetMaxHealth())),
		FText::AsNumber(FMath::RoundToInt(AttributeSet->GetMana())),
		FText::AsNumber(FMath::RoundToInt(AttributeSet->GetMaxMana())),
		FText::AsNumber(FMath::RoundToInt(AttributeSet->GetAttackPower())),
		FText::AsNumber(FMath::RoundToInt(AttributeSet->GetDefense()))
	);

	AttributeText->SetText(Text);
}

void AGASLesson1Character::ShowAttributeMessage(const FString& Message, const FColor& Color) const
{
	const FString FullMessage = FString::Printf(TEXT("[GASLesson1] %s"), *Message);
	UE_LOG(LogTemp, Warning, TEXT("%s"), *FullMessage);

	if (GEngine && IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.5f, Color, FullMessage);
	}
}

#undef LOCTEXT_NAMESPACE
