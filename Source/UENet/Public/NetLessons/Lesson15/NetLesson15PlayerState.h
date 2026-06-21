// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NetLesson15PlayerState.generated.h"

UCLASS()
class UENET_API ANetLesson15PlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetTeamId(int32 InTeamId);
	int32 GetTeamId() const;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_TeamId, VisibleAnywhere, Category = "Network Lesson 15")
	int32 TeamId = 0;

	UFUNCTION()
	void OnRep_TeamId();
};
