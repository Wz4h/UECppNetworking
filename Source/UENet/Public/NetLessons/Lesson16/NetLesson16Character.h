// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetLesson16Character.generated.h"

class UCameraComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS()
class UENET_API ANetLesson16Character : public AActor
{
	GENERATED_BODY()

public:
	ANetLesson16Character();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void RequestMoveFromServer();
	void RequestResetFromServer();
	UCameraComponent* GetDemoCamera() const;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Net Lesson 16")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Net Lesson 16")
	UStaticMeshComponent* DemoMesh;

	UPROPERTY(VisibleAnywhere, Category = "Net Lesson 16")
	UTextRenderComponent* DebugText;

	UPROPERTY(VisibleAnywhere, Category = "Net Lesson 16")
	UCameraComponent* Camera;

	// 服务器权威位置，客户端只通过复制值更新位置。
	UPROPERTY(ReplicatedUsing = OnRep_ServerLocation, VisibleAnywhere, Category = "Net Lesson 16")
	FVector ServerLocation = FVector::ZeroVector;

	// 服务器权威大小，客户端只通过复制值更新大小。
	UPROPERTY(ReplicatedUsing = OnRep_ServerScale, VisibleAnywhere, Category = "Net Lesson 16")
	float ServerScale = 1.f;

	// 手动模拟服务器移动结果返回客户端前的下行延迟。
	UPROPERTY(VisibleAnywhere, Category = "Net Lesson 16")
	float DownstreamDelaySeconds = 1.f;

	UPROPERTY(VisibleAnywhere, Category = "Net Lesson 16")
	float MoveDistance = 150.f;

	UPROPERTY(VisibleAnywhere, Category = "Net Lesson 16")
	float ScaleStep = 0.25f;

	FTimerHandle DelayedMoveTimerHandle;
	FVector PendingReplicatedLocation = FVector::ZeroVector;
	float PendingReplicatedScale = 1.f;

	UFUNCTION()
	void OnRep_ServerLocation();

	UFUNCTION()
	void OnRep_ServerScale();

	void ReplicateDelayedServerLocation();
	void ApplyServerLocation();
	void ApplyServerScale();
	void SetServerLocation(const FVector& NewLocation);
	void SetServerScale(float NewScale);
	void UpdateDebugText();
	void ShowEventMessage(const FString& Message, const FColor& Color = FColor::Yellow) const;
	FString NetRoleToString(ENetRole InRole) const;
};
