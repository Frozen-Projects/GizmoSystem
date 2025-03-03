// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Components/PrimitiveComponent.h"

#include "CustomCollision.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), ShowCategories = ("Mobility", "Transform", "Collision"))
class GIZMOSYSTEM_API UCustomCollision : public UPrimitiveComponent
{
    GENERATED_BODY()

public:

    // Constructor using FObjectInitializer for proper default subobject creation.
    UCustomCollision(const FObjectInitializer& ObjectInitializer);

    // Override to update collision data once the component is registered.
    virtual void OnRegister() override;

    // Create a basic scene proxy so you can visualize the box in the viewport.
    virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

    // Calculate the component’s bounds based on the corner vertices.
    virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

    virtual UBodySetup* GetBodySetup() override;

    /** Update the collision geometry using the current corner positions */
    void UpdateCollision();

    /**
    * Array of 8 vertices representing the corners of your box.
    * Order convention:
    * 0: (-X, -Y, -Z)  1: (+X, -Y, -Z)  2: (+X, +Y, -Z)  3: (-X, +Y, -Z)
    * 4: (-X, -Y, +Z)  5: (+X, -Y, +Z)  6: (+X, +Y, +Z)  7: (-X, +Y, +Z)
    */
    UPROPERTY(EditAnywhere, Category = "Custom Collision")
    TArray<FVector> Corners;

    UFUNCTION(BlueprintCallable, Category = "Custom Collision")
    bool SetExtents(TArray<FVector> New_Corners);

#if WITH_EDITOR

    // Called whenever a property is changed in the editor.
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

#endif

protected:

    FVector Default_Extents = FVector(50.f, 50.f, 50.f);
    UBodySetup* CustomBodySetup;

};

// A scene proxy that visualizes the custom box collision with debug lines.
class FCustomBoxSceneProxy : public FPrimitiveSceneProxy
{
public:
    
    TArray<FVector> BoxVertices;

    FCustomBoxSceneProxy(const UCustomCollision* InComponent): FPrimitiveSceneProxy(InComponent), BoxVertices(InComponent->Corners) { }

    virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;
    virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;
    virtual SIZE_T GetTypeHash() const override;
    virtual uint32 GetMemoryFootprint() const override;

};