// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GASLesson2EffectLibrary.generated.h"

class UGameplayEffect;
class UCurveTable;

/** 为编辑器自动化提供 UE5.6 GameplayEffect Component 的最小配置入口。 */
UCLASS()
class UENET_API UGASLesson2EffectLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** 在 GameplayEffect 上创建 Target Tags Component，并写入 Granted Tags。 */
	UFUNCTION(BlueprintCallable, Category = "GAS Lesson 2|Editor")
	static void ConfigureGrantedTags(UGameplayEffect* GameplayEffect, const FGameplayTagContainer& GrantedTags);

	/** 供资产生成脚本校验 GameplayEffect 最终授予的标签。 */
	UFUNCTION(BlueprintPure, Category = "GAS Lesson 2|Editor")
	static bool HasGrantedTag(const UGameplayEffect* GameplayEffect, FGameplayTag GrantedTag);

	/** 在 GameplayEffect 上创建 Target Tag Requirements Component，并配置应用所需标签。 */
	UFUNCTION(BlueprintCallable, Category = "GAS Lesson 2|Editor")
	static void ConfigureApplicationRequiredTags(UGameplayEffect* GameplayEffect, const FGameplayTagContainer& RequiredTags);

	/** 供资产生成脚本校验 Application Tag Requirements。 */
	UFUNCTION(BlueprintPure, Category = "GAS Lesson 2|Editor")
	static bool HasApplicationRequiredTag(const UGameplayEffect* GameplayEffect, FGameplayTag RequiredTag);

	/** 配置 GE 资产自身的标签；这些标签不会授予目标 Actor。 */
	UFUNCTION(BlueprintCallable, Category = "GAS Lesson 2|Editor")
	static void ConfigureAssetTags(UGameplayEffect* GameplayEffect, const FGameplayTagContainer& AssetTags);

	UFUNCTION(BlueprintPure, Category = "GAS Lesson 2|Editor")
	static bool HasAssetTag(const UGameplayEffect* GameplayEffect, FGameplayTag AssetTag);

	/** 配置 Remove Other Effects Component，使该 GE 清除带指定 Asset Tag 的持续效果。 */
	UFUNCTION(BlueprintCallable, Category = "GAS Lesson 2|Editor")
	static void ConfigureRemoveEffectsWithAssetTags(UGameplayEffect* GameplayEffect, const FGameplayTagContainer& AssetTags);

	/** 配置 Immunity Component，使该持续 GE 阻止带指定 Asset Tag 的新效果。 */
	UFUNCTION(BlueprintCallable, Category = "GAS Lesson 2|Editor")
	static void ConfigureImmunityToAssetTags(UGameplayEffect* GameplayEffect, const FGameplayTagContainer& AssetTags);

	UFUNCTION(BlueprintPure, Category = "GAS Lesson 2|Editor")
	static bool HasRemoveEffectsQueryForAssetTag(const UGameplayEffect* GameplayEffect, FGameplayTag AssetTag);

	UFUNCTION(BlueprintPure, Category = "GAS Lesson 2|Editor")
	static bool HasImmunityQueryForAssetTag(const UGameplayEffect* GameplayEffect, FGameplayTag AssetTag);

	/** 把 Health Modifier 配置为目标 MaxHealth 的 25%。 */
	UFUNCTION(BlueprintCallable, Category = "GAS Lesson 2|Editor")
	static void ConfigureAttributeBasedHealModifier(UGameplayEffect* GameplayEffect);

	/** 把 Health Modifier 配置为 UGASLesson2AttackHealMMC 的返回值。 */
	UFUNCTION(BlueprintCallable, Category = "GAS Lesson 2|Editor")
	static void ConfigureMMCHealModifier(UGameplayEffect* GameplayEffect);

	/** 添加 UGASLesson2DamageExecution，并清空普通 Modifiers。 */
	UFUNCTION(BlueprintCallable, Category = "GAS Lesson 2|Editor")
	static void ConfigureDamageExecution(UGameplayEffect* GameplayEffect);

	/** 写入 F2 案例使用的 Level 1/2/3 伤害曲线。 */
	UFUNCTION(BlueprintCallable, Category = "GAS Lesson 2|Editor")
	static void ConfigureLevelDamageCurve(UCurveTable* CurveTable);

	/** 把 Health Modifier 配置为 -1 * Curve[GameplayEffect Level]。 */
	UFUNCTION(BlueprintCallable, Category = "GAS Lesson 2|Editor")
	static void ConfigureLevelDamageModifier(UGameplayEffect* GameplayEffect, UCurveTable* CurveTable);

	/** 供资产生成脚本验证曲线在指定等级计算出的最终伤害值。 */
	UFUNCTION(BlueprintPure, Category = "GAS Lesson 2|Editor")
	static float GetLevelDamageMagnitude(const UCurveTable* CurveTable, float Level);
};
