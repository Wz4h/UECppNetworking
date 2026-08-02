// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetLesson19RelevancyCube.generated.h"

class UStaticMeshComponent;

/**
 * Relevancy 课程用的网络 Cube。
 * 这个 Actor 本身只负责被复制，具体 AlwaysRelevant / CullDistance 配置由 Spawner 在服务器生成后设置。
 */
UCLASS()
class UENET_API ANetLesson19RelevancyCube : public AActor
{
	GENERATED_BODY()

public:
	/** 构造函数：创建 Mesh，并开启 Actor 复制。 */
	ANetLesson19RelevancyCube();

protected:
	/** 课程演示用的静态网格组件，默认使用引擎 Cube。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Lesson 19")
	UStaticMeshComponent* MeshComponent;
};
