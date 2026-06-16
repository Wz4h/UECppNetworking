// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetLesson12RelevancyActor.generated.h"

class UStaticMeshComponent;
class UTextRenderComponent;

UENUM()
enum class ENetLesson12RelevancyMode : uint8
{
	Distance,
	Always,
	OwnerOnly,
	TeamOnly
};

UCLASS()
class UENET_API ANetLesson12RelevancyActor : public AActor
{
	GENERATED_BODY()

public:
	ANetLesson12RelevancyActor();

	virtual void BeginPlay() override;
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void ConfigureDemoActor(ENetLesson12RelevancyMode InMode, const FString& InName, const FString& InRuleText, int32 InColorIndex, int32 InTeamId = 0);
	void AddValueOnServer();
	void SetTeamIdOnServer(int32 InTeamId);
	int32 GetTeamId() const;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Network Lesson 12")
	UStaticMeshComponent* DemoMesh;

	UPROPERTY(VisibleAnywhere, Category = "Network Lesson 12")
	UTextRenderComponent* DemoText;

	UPROPERTY(ReplicatedUsing = OnRep_Visual, VisibleAnywhere, Category = "Network Lesson 12")
	ENetLesson12RelevancyMode Mode = ENetLesson12RelevancyMode::Distance;

	UPROPERTY(ReplicatedUsing = OnRep_Visual, VisibleAnywhere, Category = "Network Lesson 12")
	FString DemoName = TEXT("Relevancy Actor");

	UPROPERTY(ReplicatedUsing = OnRep_Visual, VisibleAnywhere, Category = "Network Lesson 12")
	FString RuleText = TEXT("Relevant");

	UPROPERTY(ReplicatedUsing = OnRep_Visual, VisibleAnywhere, Category = "Network Lesson 12")
	int32 ColorIndex = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Visual, VisibleAnywhere, Category = "Network Lesson 12")
	int32 DemoValue = 1;

	UPROPERTY(ReplicatedUsing = OnRep_Visual, VisibleAnywhere, Category = "Network Lesson 12")
	int32 TeamId = 0;

	UFUNCTION()
	void OnRep_Visual();

	void UpdateVisual();
	int32 GetViewerTeamId(const AActor* RealViewer, const AActor* ViewTarget) const;
	FLinearColor GetDemoColor() const;
};
