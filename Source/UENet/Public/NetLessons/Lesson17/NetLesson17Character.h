// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetLesson17Character.generated.h"

class UCameraComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS()
class UENET_API ANetLesson17Character : public AActor
{
	GENERATED_BODY()

public:
	ANetLesson17Character();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 客户端预测用：只修改当前机器上的显示大小。
	void PredictGrowLocally();

	// 服务端权威用：由 PlayerController 的 Server RPC 调用。
	void RequestGrowFromServer();

	// 服务端重置权威大小。
	void RequestResetFromServer();

	UCameraComponent* GetDemoCamera() const;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Net Lesson 17")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Net Lesson 17")
	UStaticMeshComponent* DemoMesh;

	UPROPERTY(VisibleAnywhere, Category = "Net Lesson 17")
	UTextRenderComponent* DebugText;

	UPROPERTY(VisibleAnywhere, Category = "Net Lesson 17")
	UCameraComponent* Camera;

	// 服务器权威大小，客户端最终会收到这个值。
	UPROPERTY(ReplicatedUsing = OnRep_ServerScale, VisibleAnywhere, Category = "Net Lesson 17")
	float ServerScale = 1.f;

	// 客户端本地预测显示大小，不参与复制。
	float LocalPredictedScale = 1.f;

	// 客户端预测步长，故意更大，用来制造预测误差。
	UPROPERTY(VisibleAnywhere, Category = "Net Lesson 17")
	float ClientPredictionScaleStep = 0.75f;

	// 服务器权威步长，故意更小，用来演示服务器纠正。
	UPROPERTY(VisibleAnywhere, Category = "Net Lesson 17")
	float ServerAuthorityScaleStep = 0.25f;

	// 服务器执行后等待一段时间，再把权威值复制给客户端。
	UPROPERTY(VisibleAnywhere, Category = "Net Lesson 17")
	float ServerCorrectionDelaySeconds = 1.f;

	FTimerHandle DelayedCorrectionTimerHandle;
	float PendingServerScale = 1.f;

	UFUNCTION()
	void OnRep_ServerScale();

	void ReplicateDelayedServerScale();
	void ApplyScale(float NewScale);
	void SetServerScale(float NewScale);
	void UpdateDebugText();
	void ShowEventMessage(const FString& Message, const FColor& Color = FColor::Yellow) const;
	FString NetRoleToString(ENetRole InRole) const;
};
