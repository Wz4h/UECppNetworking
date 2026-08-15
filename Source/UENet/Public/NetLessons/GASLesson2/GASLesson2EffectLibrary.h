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
