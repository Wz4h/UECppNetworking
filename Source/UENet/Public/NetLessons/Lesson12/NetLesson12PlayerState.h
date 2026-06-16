// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NetLesson12PlayerState.generated.h"

UCLASS()
class UENET_API ANetLesson12PlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetTeamId(int32 InTeamId);
	int32 GetTeamId() const;

protected:
	// 演示用队伍编号，自定义 Relevancy 会读取它。
	UPROPERTY(ReplicatedUsing = OnRep_TeamId, VisibleAnywhere, Category = "Network Lesson 12")
	int32 TeamId = 0;

	UFUNCTION()
	void OnRep_TeamId();
};
