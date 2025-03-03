// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Gizmo_Includes.h"
#include "Gizmo_Enums.h"

#include "Gizmo_Trace_Move.generated.h"

UCLASS()
class GIZMOSYSTEM_API AGizmoTraceMove : public AActor
{
    GENERATED_BODY()

protected:

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:

    AGizmoTraceMove();
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere)
    USceneComponent* Root;

    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
    USceneComponent* Target;

    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
    UStaticMeshComponent* XAxis;

    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
    UStaticMeshComponent* YAxis;

    UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
    UStaticMeshComponent* ZAxis;

};