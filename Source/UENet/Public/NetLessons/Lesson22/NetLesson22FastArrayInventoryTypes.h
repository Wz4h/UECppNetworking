// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "NetLesson22FastArrayInventoryTypes.generated.h"

struct FNetLesson22FastItemArray;

/**
 * FastArray 里的单个物品项。
 * 继承 FFastArraySerializerItem 后，复制系统可以只同步发生变化的元素。
 */
USTRUCT(BlueprintType)
struct UENET_API FNetLesson22FastItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	/** 物品 ID，课堂中只作为日志识别用。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Lesson 22")
	int32 ItemId = 0;

	/** 物品数量，修改这个字段后需要 MarkItemDirty。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Lesson 22")
	int32 Count = 0;

	/** 客户端新增元素后的日志回调。 */
	void PostReplicatedAdd(const FNetLesson22FastItemArray& InArraySerializer);

	/** 客户端元素发生变化后的日志回调。 */
	void PostReplicatedChange(const FNetLesson22FastItemArray& InArraySerializer);

	/** 客户端元素被移除前的日志回调。 */
	void PreReplicatedRemove(const FNetLesson22FastItemArray& InArraySerializer);
};

/**
 * FastArray 容器。
 * Items 通过 NetDeltaSerialize 走增量同步，而不是普通 TArray 的整体同步路径。
 */
USTRUCT(BlueprintType)
struct UENET_API FNetLesson22FastItemArray : public FFastArraySerializer
{
	GENERATED_BODY()

	/** 需要增量同步的物品列表。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Lesson 22")
	TArray<FNetLesson22FastItem> Items;

	/** FastArray 增量序列化入口。 */
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FNetLesson22FastItem, FNetLesson22FastItemArray>(Items, DeltaParms, *this);
	}
};

/** 告诉 UE 反射系统 FNetLesson22FastItemArray 使用 NetDeltaSerialize。 */
template<>
struct TStructOpsTypeTraits<FNetLesson22FastItemArray> : public TStructOpsTypeTraitsBase2<FNetLesson22FastItemArray>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
};
