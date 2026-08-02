// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/Interactable.h"
#include "InteractionDoor.generated.h"

class UStaticMeshComponent;

// 联机交互门示例；只通过 IInteractable 接口响应服务器交互请求，并用复制变量同步开关状态。
UCLASS(Blueprintable)
class ONLINEINTERACTION_API AInteractionDoor : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	// 创建门 Actor，初始化网格组件并开启 Actor 复制。
	AInteractionDoor();

	// 返回门是否允许当前交互者交互；示例中始终允许。
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;

	// 服务器验证通过后切换门的打开状态，并触发本地旋转更新。
	virtual void Interact_Implementation(AActor* Interactor) override;

	// 返回门当前状态对应的基础交互提示信息。
	virtual FInteractionInfo GetInteractionInfo_Implementation() const override;

	// 注册需要网络同步的属性。
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// 门的静态网格组件，蓝图中可替换具体 Mesh。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction")
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	// 门当前是否处于打开状态；服务器修改后复制到客户端并调用 OnRep_IsOpen。
	UPROPERTY(ReplicatedUsing=OnRep_IsOpen, BlueprintReadOnly, Category="Interaction")
	bool bIsOpen = false;

	// 门关闭时使用的旋转。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	FRotator ClosedRotation = FRotator(0.f, 0.f, 0.f);

	// 门打开时使用的旋转。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	FRotator OpenRotation = FRotator(0.f, 90.f, 0.f);

	// 当开关状态复制到客户端时应用门旋转。
	UFUNCTION()
	void OnRep_IsOpen();

private:
	// 根据 bIsOpen 应用关闭或打开旋转。
	void ApplyDoorRotation();
};
