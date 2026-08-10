// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GASLesson2EffectLibrary.generated.h"

class UGameplayEffect;

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
};
