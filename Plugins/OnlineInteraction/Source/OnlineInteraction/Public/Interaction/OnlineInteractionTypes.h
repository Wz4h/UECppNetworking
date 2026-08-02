// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineInteractionTypes.generated.h"

// 交互请求的通用结果类型，后续交互流程可根据该结果决定提示或后续处理。
UENUM(BlueprintType)
enum class EInteractionResult : uint8
{
	// 交互成功。
	Success,

	// 交互失败，表示通用失败原因。
	Failed,

	// 交互目标无效。
	InvalidTarget,

	// 交互目标超出允许距离。
	OutOfRange
};

// 交互显示和规则所需的基础信息；当前仅作为数据容器，后续可供 UI 或检测逻辑使用。
USTRUCT(BlueprintType)
struct ONLINEINTERACTION_API FInteractionInfo
{
	GENERATED_BODY()

public:
	// 显示给玩家的交互提示文本。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	FText InteractionText;

	// 允许交互的最大距离。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	float MaxDistance = 300.f;
};
