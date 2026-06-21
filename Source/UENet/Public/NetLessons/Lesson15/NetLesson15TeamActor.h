// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetLesson15TeamActor.generated.h"

class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS()
class UENET_API ANetLesson15TeamActor : public AActor
{
	GENERATED_BODY()

public:
	ANetLesson15TeamActor();

	virtual void BeginPlay() override;
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void ToggleTeamOnServer();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Network Lesson 15")
	UStaticMeshComponent* DemoMesh;

	UPROPERTY(VisibleAnywhere, Category = "Network Lesson 15")
	UTextRenderComponent* DemoText;

	UPROPERTY(ReplicatedUsing = OnRep_Visual, VisibleAnywhere, Category = "Network Lesson 15")
	int32 TeamId = 1;

	UPROPERTY(ReplicatedUsing = OnRep_Visual, VisibleAnywhere, Category = "Network Lesson 15")
	int32 Value = 1;

	UFUNCTION()
	void OnRep_Visual();

	void UpdateVisual();
	int32 GetViewerTeamId(const AActor* RealViewer, const AActor* ViewTarget) const;
};
