// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetLessons/Lesson22/NetLesson22FastArrayInventoryTypes.h"
#include "NetLesson22FastArrayInventoryActor.generated.h"

/**
 * FastArray 课程用的背包 Actor。
 * 同时持有普通 TArray 和 FastArray，用于对比“整体数组复制”和“变化项增量复制”。
 */
UCLASS()
class UENET_API ANetLesson22FastArrayInventoryActor : public AActor
{
	GENERATED_BODY()

public:
	/** 构造函数：开启 Actor 复制。 */
	ANetLesson22FastArrayInventoryActor();

	/** 游戏开始时只在服务器初始化两份列表数据。 */
	virtual void BeginPlay() override;

	/** 注册普通数组和 FastArray 的复制规则。 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 服务器 RPC：修改普通数组里的一个元素。 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Network Lesson 22")
	void ServerModifyNormalItem(int32 Index, int32 NewValue);

	/** 服务器 RPC：修改 FastArray 里的一个元素，并 MarkItemDirty。 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Network Lesson 22")
	void ServerModifyFastItem(int32 Index, int32 NewCount);

protected:
	/** 普通数组：元素变化后客户端只知道整个数组 OnRep 了。 */
	UPROPERTY(ReplicatedUsing = OnRep_NormalItems, VisibleAnywhere, Category = "Network Lesson 22")
	TArray<int32> NormalItems;

	/** FastArray：元素变化后客户端可以收到具体变化项的回调。 */
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Network Lesson 22")
	FNetLesson22FastItemArray FastItems;

	/** 初始化的元素数量，默认 100。 */
	UPROPERTY(EditAnywhere, Category = "Network Lesson 22", meta = (ClampMin = "1"))
	int32 InitialItemCount = 100;

	/** 普通数组复制到客户端后的调试日志。 */
	UFUNCTION()
	void OnRep_NormalItems();

private:
	/** 服务器初始化普通数组和 FastArray。 */
	void InitializeItems();
};
