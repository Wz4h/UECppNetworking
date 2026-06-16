// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetLesson11FrameworkActor.generated.h"

class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS()
class UENET_API ANetLesson11FrameworkActor : public AActor
{
	GENERATED_BODY()

public:
	ANetLesson11FrameworkActor();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 服务器初始化演示物体的名称、颜色和说明。
	void ConfigureDemoActor(const FString& InDemoName, int32 InColorIndex, const FString& InChannelHint);

	// 服务器修改复制数值，用于演示属性通过 ActorChannel 同步。
	void AddDemoValueOnServer();

	// 服务器移动物体，用于演示 Relevancy 会影响客户端是否保留这个 Actor。
	void SetServerLocation(const FVector& NewLocation);

protected:
	UPROPERTY(VisibleAnywhere, Category = "Network Lesson 11")
	UStaticMeshComponent* DemoMesh;

	UPROPERTY(VisibleAnywhere, Category = "Network Lesson 11")
	UTextRenderComponent* DemoText;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Network Lesson 11")
	FString DemoName = TEXT("Actor");

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Network Lesson 11")
	FString ChannelHint = TEXT("ActorChannel");

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Network Lesson 11")
	int32 ColorIndex = 0;

	UPROPERTY(ReplicatedUsing = OnRep_DemoValue, VisibleAnywhere, Category = "Network Lesson 11")
	int32 DemoValue = 1;

	UFUNCTION()
	void OnRep_DemoValue();

	// 根据复制数值刷新 Mesh 大小和文字。
	void UpdateVisual();

	FLinearColor GetDemoColor() const;
};
