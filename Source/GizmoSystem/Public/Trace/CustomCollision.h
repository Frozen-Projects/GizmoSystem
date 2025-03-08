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

    UCustomCollision(const FObjectInitializer& ObjectInitializer);

    virtual void OnRegister() override;

    virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
    virtual UBodySetup* GetBodySetup() override;
    virtual void UpdateBodySetup() override;
    void UpdateCollision();

    virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    UPROPERTY(EditAnywhere, Category = "Custom Collision")
    TArray<FVector> Corners;

    UFUNCTION(BlueprintCallable, Category = "Custom Collision")
    bool SetExtents(TArray<FVector> New_Corners);

    UFUNCTION(BlueprintCallable, Category = "Custom Collision")
    virtual float GetLineThickness() const;

protected:

    FVector Default_Extents = FVector(50.f, 50.f, 50.f);
    
    UPROPERTY(Transient)
    UBodySetup* CustomBodySetup = nullptr;

};

// A scene proxy that visualizes the custom box collision with debug lines.
class FCustomBoxSceneProxy : public FPrimitiveSceneProxy
{
public:

    TArray<FVector> BoxVertices;
    const UCustomCollision* Component;

    FCustomBoxSceneProxy(const UCustomCollision* InComponent);
    virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;
    virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;
    virtual SIZE_T GetTypeHash() const override;
    virtual uint32 GetMemoryFootprint() const override;

};