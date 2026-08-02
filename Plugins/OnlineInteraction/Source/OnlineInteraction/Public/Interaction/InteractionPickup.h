// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/Interactable.h"
#include "InteractionPickup.generated.h"

class UStaticMeshComponent;

// 联机拾取物示例；服务器交互后销毁自身，并通过 Actor 销毁复制让所有客户端同步消失。
UCLASS(Blueprintable)
class ONLINEINTERACTION_API AInteractionPickup : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	// 创建拾取物 Actor，初始化网格组件并开启 Actor 复制。
	AInteractionPickup();

	// 返回拾取物是否允许当前交互者交互；示例中始终允许。
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;

	// 服务器验证通过后销毁拾取物。
	virtual void Interact_Implementation(AActor* Interactor) override;

	// 返回拾取物的基础交互提示信息。
	virtual FInteractionInfo GetInteractionInfo_Implementation() const override;

protected:
	// 拾取物的静态网格组件，蓝图中可替换具体 Mesh。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction")
	TObjectPtr<UStaticMeshComponent> PickupMesh;
};
