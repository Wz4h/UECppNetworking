// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "InteractionComponent.generated.h"

// 玩家或其他 Actor 可挂载的交互组件，负责查找当前目标并通过服务器 RPC 发起交互请求。
UCLASS(ClassGroup=(Interaction), meta=(BlueprintSpawnableComponent))
class ONLINEINTERACTION_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// 创建交互组件，关闭 Tick，并允许组件在拥有者具备网络所有权时发送服务器 RPC。
	UInteractionComponent();

	// 从拥有者视角向前检测可交互目标的最大距离。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	float TraceDistance = 500.f;

	// 检测可交互目标时使用的碰撞通道。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	// 当前客户端视角下命中的可交互目标；服务器仍会在交互时重新验证。
	UPROPERTY(BlueprintReadOnly, Category="Interaction")
	AActor* CurrentTarget = nullptr;

	// 从拥有者视角执行 LineTrace，并更新当前可交互目标。
	UFUNCTION(BlueprintCallable, Category="Interaction")
	void UpdateInteractionTarget();

	// 请求与当前目标交互；客户端只发送服务器 RPC，不直接执行交互逻辑。
	UFUNCTION(BlueprintCallable, Category="Interaction")
	void TryInteract();

protected:
	// 在服务器上处理交互请求，所有真正的交互调用都必须经过这里验证。
	UFUNCTION(Server, Reliable)
	void ServerInteract(AActor* Target);

	// 判断目标是否存在并实现了可交互接口。
	bool IsValidInteractionTarget(AActor* Target) const;

	// 判断目标是否仍在允许交互的距离范围内。
	bool IsTargetInRange(AActor* Target) const;
};
