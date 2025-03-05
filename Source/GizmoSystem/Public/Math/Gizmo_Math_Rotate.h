// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Gizmo_Math_Base.h"

#include "Gizmo_Math_Rotate.generated.h"

UCLASS()
class GIZMOSYSTEM_API AGizmoMathRotate : public AActor
{
	GENERATED_BODY()
	
protected:
	
	// Called when the game starts or when spawned.
	virtual void BeginPlay() override;

	// Called when the game end or when destroyed.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	APlayerController* PlayerController = nullptr;

	virtual void InitHandles();

	virtual bool Rotate_Check();
	virtual bool Check_Visibility();

	virtual void RotateSystem();
	virtual FVector HorizontalNormal(USceneComponent* Target);
	virtual double Rotate_XY();

public:	

	// Sets default values for this actor's properties.
	AGizmoMathRotate();

	// Called every frame.
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root = nullptr;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Axis_X = nullptr;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Axis_Y = nullptr;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Axis_Z = nullptr;

	UPROPERTY(BlueprintReadOnly)
	AGizmoMathBase* GizmoBase = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UPrimitiveComponent* AxisComponent = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	ESelectedAxis AxisEnum = ESelectedAxis::Null_Axis;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bRotateLocal = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float RotateMultiplier = 5;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = "true"))
	bool bEnableDebugMode = false;

};
