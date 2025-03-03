// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Gizmo_Includes.h"
#include "Gizmo_Enums.h"

#include "Gizmo_Math_Base.generated.h"

UCLASS()
class GIZMOSYSTEM_API AGizmoMathBase : public AActor
{
	GENERATED_BODY()
	
protected:
	
	// Called when the game starts or when spawned.
	virtual void BeginPlay() override;

	// Called when the game end or when destroyed.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	TSet<FKey> PressedKeys;
	APlayerController* PlayerController = nullptr;
	UCapsuleComponent* CapsuleComponent = nullptr;

public:	

	// Sets default values for this actor's properties.
	AGizmoMathBase();

	// Called every frame.
	virtual void Tick(float DeltaTime) override;

// Callbacks.
public:

	virtual void AnyKey_Pressed(FKey Key);
	virtual void AnyKey_Released(FKey Key);
	virtual bool ForbiddenKeysCallback();
	virtual bool DetectMovementCallback();
	virtual bool IsGizmoInViewCallback();

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* GizmoType = nullptr;

	UPROPERTY(BlueprintReadWrite)
	UCameraComponent* PlayerCamera = nullptr;

	UPROPERTY(BlueprintReadWrite)
	int32 PlayerIndex = 0;

	UPROPERTY(BlueprintReadWrite)
	USceneComponent* GizmoTarget = nullptr;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 GizmoSizeMultiplier = 1150;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FKey> ForbiddenKeys;

};
