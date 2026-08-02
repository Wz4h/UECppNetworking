// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetLessons/Lesson22/NetLesson22FastArrayInventoryActor.h"

#include "Net/UnrealNetwork.h"

void FNetLesson22FastItem::PostReplicatedAdd(const FNetLesson22FastItemArray& InArraySerializer)
{
	UE_LOG(LogTemp, Warning, TEXT("FastItem Added: ItemId=%d Count=%d"), ItemId, Count);
}

void FNetLesson22FastItem::PostReplicatedChange(const FNetLesson22FastItemArray& InArraySerializer)
{
	UE_LOG(LogTemp, Warning, TEXT("FastItem Changed: ItemId=%d Count=%d"), ItemId, Count);
}

void FNetLesson22FastItem::PreReplicatedRemove(const FNetLesson22FastItemArray& InArraySerializer)
{
	UE_LOG(LogTemp, Warning, TEXT("FastItem Removed: ItemId=%d Count=%d"), ItemId, Count);
}

ANetLesson22FastArrayInventoryActor::ANetLesson22FastArrayInventoryActor()
{
	bReplicates = true;
	SetReplicateMovement(false);
}

void ANetLesson22FastArrayInventoryActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		InitializeItems();
	}
}

void ANetLesson22FastArrayInventoryActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetLesson22FastArrayInventoryActor, NormalItems);
	DOREPLIFETIME(ANetLesson22FastArrayInventoryActor, FastItems);
}

void ANetLesson22FastArrayInventoryActor::ServerModifyNormalItem_Implementation(int32 Index, int32 NewValue)
{
	if (!NormalItems.IsValidIndex(Index))
	{
		return;
	}

	NormalItems[Index] = NewValue;
	UE_LOG(LogTemp, Warning, TEXT("Server Modified NormalItems[%d]=%d"), Index, NewValue);
}

void ANetLesson22FastArrayInventoryActor::ServerModifyFastItem_Implementation(int32 Index, int32 NewCount)
{
	if (!FastItems.Items.IsValidIndex(Index))
	{
		return;
	}

	FNetLesson22FastItem& Item = FastItems.Items[Index];
	Item.Count = NewCount;
	FastItems.MarkItemDirty(Item);

	UE_LOG(LogTemp, Warning, TEXT("Server Modified FastItems[%d]: ItemId=%d Count=%d"), Index, Item.ItemId, Item.Count);
}

void ANetLesson22FastArrayInventoryActor::OnRep_NormalItems()
{
	UE_LOG(LogTemp, Warning, TEXT("NormalItems Replicated"));
}

void ANetLesson22FastArrayInventoryActor::InitializeItems()
{
	NormalItems.Reset();
	FastItems.Items.Reset();

	for (int32 Index = 0; Index < InitialItemCount; ++Index)
	{
		NormalItems.Add(Index);

		FNetLesson22FastItem& NewItem = FastItems.Items.AddDefaulted_GetRef();
		NewItem.ItemId = Index;
		NewItem.Count = Index;
		FastItems.MarkItemDirty(NewItem);
	}

	FastItems.MarkArrayDirty();
}
