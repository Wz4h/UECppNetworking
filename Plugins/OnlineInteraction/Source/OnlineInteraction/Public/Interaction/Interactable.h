// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/OnlineInteractionTypes.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

// 可交互对象的统一蓝图接口类型，用于让 Actor 或组件声明自己支持交互能力。
UINTERFACE(BlueprintType)
class ONLINEINTERACTION_API UInteractable : public UInterface
{
	GENERATED_BODY()
};

// 可交互对象的 C++ 接口声明；只定义通用交互协议，不包含 Door、Pickup 等具体业务逻辑。
class ONLINEINTERACTION_API IInteractable
{
	GENERATED_BODY()

public:
	// 判断指定交互发起者当前是否允许与该对象交互。
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	bool CanInteract(AActor* Interactor) const;

	// 在服务器验证通过后执行交互行为，具体业务由实现该接口的对象决定。
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	void Interact(AActor* Interactor);

	// 获取交互提示和规则所需的基础信息，供后续 UI 或交互检测扩展使用。
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	FInteractionInfo GetInteractionInfo() const;
};
