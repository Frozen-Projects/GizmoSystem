// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Components/ShapeComponent.h"

#include "CustomCollision.generated.h"

UCLASS(ClassGroup = (Collision), meta = (BlueprintSpawnableComponent), ShowCategories = ("Mobility", "Transform", "Collision"))
class GIZMOSYSTEM_API UCustomCollision : public UShapeComponent
{
    GENERATED_BODY()

public:

    // Constructor using FObjectInitializer for proper default subobject creation.
    UCustomCollision(const FObjectInitializer& ObjectInitializer);

    virtual void OnRegister() override;
    virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
    virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

    virtual UBodySetup* GetBodySetup() override;
    virtual void UpdateBodySetup() override;
    void UpdateCollision();
    virtual float GetLineThickness() const;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    UPROPERTY(EditAnywhere, Category = "Custom Collision")
    TArray<FVector> Corners;

    UFUNCTION(BlueprintCallable, Category = "Custom Collision")
    bool SetExtents(TArray<FVector> New_Corners);

protected:

    FVector Default_Extents = FVector(50.f, 50.f, 50.f);
    UBodySetup* CustomBodySetup;

};

// A scene proxy that visualizes the custom box collision with debug lines.
class FCustomBoxSceneProxy : public FPrimitiveSceneProxy
{
public:
    
    TArray<FVector> BoxVertices;
    const UCustomCollision* Component;

    FCustomBoxSceneProxy(const UCustomCollision* InComponent)
        : FPrimitiveSceneProxy(InComponent)
        , BoxVertices(InComponent->Corners)
        , Component(InComponent)
    {
    }

    virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;
    virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;
    virtual SIZE_T GetTypeHash() const override;
    virtual uint32 GetMemoryFootprint() const override;

};