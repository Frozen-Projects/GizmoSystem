// Fill out your copyright notice in the Description page of Project Settings.

#include "Custom/CustomBoxCollision.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/ConvexElem.h"

#include "Misc/Crc.h"

#include "Engine/Engine.h"

UCustomBoxCollision::UCustomBoxCollision(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer), CustomBodySetup(nullptr) // Don't create it here; wait until we have a valid World
{
    PrimaryComponentTick.bCanEverTick = false;

    // Set default half extents (50 units in each direction for a 100x100x100 box)
    BoxHalfExtents = FVector(50.f, 50.f, 50.f);

    // Initialize the eight corners based on BoxHalfExtents.
    Corners.Empty();
    Corners.Add(FVector(-BoxHalfExtents.X, -BoxHalfExtents.Y, -BoxHalfExtents.Z));
    Corners.Add(FVector(BoxHalfExtents.X, -BoxHalfExtents.Y, -BoxHalfExtents.Z));
    Corners.Add(FVector(BoxHalfExtents.X, BoxHalfExtents.Y, -BoxHalfExtents.Z));
    Corners.Add(FVector(-BoxHalfExtents.X, BoxHalfExtents.Y, -BoxHalfExtents.Z));
    Corners.Add(FVector(-BoxHalfExtents.X, -BoxHalfExtents.Y, BoxHalfExtents.Z));
    Corners.Add(FVector(BoxHalfExtents.X, -BoxHalfExtents.Y, BoxHalfExtents.Z));
    Corners.Add(FVector(BoxHalfExtents.X, BoxHalfExtents.Y, BoxHalfExtents.Z));
    Corners.Add(FVector(-BoxHalfExtents.X, BoxHalfExtents.Y, BoxHalfExtents.Z));
}

void UCustomBoxCollision::OnRegister()
{
    Super::OnRegister();

    // At this point, GetWorld() should be valid.
    if (GetWorld())
    {
        // Always create a new UBodySetup with Outer set to GetWorld().
        CustomBodySetup = NewObject<UBodySetup>(GetWorld(), UBodySetup::StaticClass(), TEXT("CustomBoxCollision_BodySetup"));
        if (CustomBodySetup)
        {
            CustomBodySetup->CollisionTraceFlag = CTF_UseDefault;
        }
    }
    else
    {
        // Fallback if GetWorld() isn't valid (should rarely happen).
        if (!CustomBodySetup)
        {
            CustomBodySetup = NewObject<UBodySetup>(this, UBodySetup::StaticClass(), TEXT("CustomBoxCollision_BodySetup"));
            if (CustomBodySetup)
            {
                CustomBodySetup->CollisionTraceFlag = CTF_UseDefault;
            }
        }
    }

    // Update collision geometry using the newly created body setup.
    UpdateCollision();
}

FPrimitiveSceneProxy* UCustomBoxCollision::CreateSceneProxy()
{
    return new FCustomBoxSceneProxy(this);
}

FBoxSphereBounds UCustomBoxCollision::CalcBounds(const FTransform& LocalToWorld) const
{
    FBox Box(ForceInit);
    for (const FVector& Vertex : Corners)
    {
        Box += Vertex;
    }

    return FBoxSphereBounds(Box.TransformBy(LocalToWorld));
}

UBodySetup* UCustomBoxCollision::GetBodySetup()
{
    return CustomBodySetup;
}

void UCustomBoxCollision::UpdateCollision()
{
    if (!CustomBodySetup)
    {
        return;
    }

    // Clear any previous collision geometry.
    CustomBodySetup->AggGeom.ConvexElems.Empty();

    // Create a convex element from the current corners.
    FKConvexElem ConvexElem;
    ConvexElem.VertexData = Corners;
    ConvexElem.UpdateElemBox(); // Updates the convex element's bounding box.
    CustomBodySetup->AggGeom.ConvexElems.Add(ConvexElem);

    // Invalidate any old physics data and rebuild the collision meshes.
    CustomBodySetup->InvalidatePhysicsData();
    CustomBodySetup->CreatePhysicsMeshes();

    // Ensure collision is enabled.
    SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Recreate the physics state so that the updated collision geometry is used.
    RecreatePhysicsState();
}

void UCustomBoxCollision::SetBoxHalfExtents(const FVector& NewHalfExtents)
{
    BoxHalfExtents = NewHalfExtents;

    // Recalculate the eight corner positions.
    if (Corners.Num() < 8)
    {
        Corners.SetNum(8);
    }

    Corners[0] = FVector(-BoxHalfExtents.X, -BoxHalfExtents.Y, -BoxHalfExtents.Z);
    Corners[1] = FVector(BoxHalfExtents.X, -BoxHalfExtents.Y, -BoxHalfExtents.Z);
    Corners[2] = FVector(BoxHalfExtents.X, BoxHalfExtents.Y, -BoxHalfExtents.Z);
    Corners[3] = FVector(-BoxHalfExtents.X, BoxHalfExtents.Y, -BoxHalfExtents.Z);
    Corners[4] = FVector(-BoxHalfExtents.X, -BoxHalfExtents.Y, BoxHalfExtents.Z);
    Corners[5] = FVector(BoxHalfExtents.X, -BoxHalfExtents.Y, BoxHalfExtents.Z);
    Corners[6] = FVector(BoxHalfExtents.X, BoxHalfExtents.Y, BoxHalfExtents.Z);
    Corners[7] = FVector(-BoxHalfExtents.X, BoxHalfExtents.Y, BoxHalfExtents.Z);

    // Update collision and refresh render state.
    UpdateCollision();
    MarkRenderStateDirty();
}

#if WITH_EDITOR
void UCustomBoxCollision::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    FName PropertyName = (PropertyChangedEvent.Property) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

    if (PropertyName == GET_MEMBER_NAME_CHECKED(UCustomBoxCollision, BoxHalfExtents))
    {
        if (Corners.Num() < 8)
        {
            Corners.SetNum(8);
        }
        Corners[0] = FVector(-BoxHalfExtents.X, -BoxHalfExtents.Y, -BoxHalfExtents.Z);
        Corners[1] = FVector(BoxHalfExtents.X, -BoxHalfExtents.Y, -BoxHalfExtents.Z);
        Corners[2] = FVector(BoxHalfExtents.X, BoxHalfExtents.Y, -BoxHalfExtents.Z);
        Corners[3] = FVector(-BoxHalfExtents.X, BoxHalfExtents.Y, -BoxHalfExtents.Z);
        Corners[4] = FVector(-BoxHalfExtents.X, -BoxHalfExtents.Y, BoxHalfExtents.Z);
        Corners[5] = FVector(BoxHalfExtents.X, -BoxHalfExtents.Y, BoxHalfExtents.Z);
        Corners[6] = FVector(BoxHalfExtents.X, BoxHalfExtents.Y, BoxHalfExtents.Z);
        Corners[7] = FVector(-BoxHalfExtents.X, BoxHalfExtents.Y, BoxHalfExtents.Z);

        UpdateCollision();
        MarkRenderStateDirty();
    }

    Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

// ----------------------------------------------------------------
// FCustomBoxSceneProxy definitions (for debug visualization)
// ----------------------------------------------------------------

void FCustomBoxSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
{

    // Get the local-to-world transform from the scene proxy.
    const FMatrix LocalToWorldMatrix = GetLocalToWorld();

    // Iterate over all scene views.
    for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
    {
        if (VisibilityMap & (1 << ViewIndex))
        {
            FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
            if (BoxVertices.Num() == 8)
            {
                // Transform vertices to world space.
                const FVector V0 = LocalToWorldMatrix.TransformPosition(BoxVertices[0]);
                const FVector V1 = LocalToWorldMatrix.TransformPosition(BoxVertices[1]);
                const FVector V2 = LocalToWorldMatrix.TransformPosition(BoxVertices[2]);
                const FVector V3 = LocalToWorldMatrix.TransformPosition(BoxVertices[3]);
                const FVector V4 = LocalToWorldMatrix.TransformPosition(BoxVertices[4]);
                const FVector V5 = LocalToWorldMatrix.TransformPosition(BoxVertices[5]);
                const FVector V6 = LocalToWorldMatrix.TransformPosition(BoxVertices[6]);
                const FVector V7 = LocalToWorldMatrix.TransformPosition(BoxVertices[7]);

                // Draw bottom face (vertices 0-1-2-3)
                PDI->DrawLine(V0, V1, FColor::Green, SDPG_World, 1.0f);
                PDI->DrawLine(V1, V2, FColor::Green, SDPG_World, 1.0f);
                PDI->DrawLine(V2, V3, FColor::Green, SDPG_World, 1.0f);
                PDI->DrawLine(V3, V0, FColor::Green, SDPG_World, 1.0f);

                // Draw top face (vertices 4-5-6-7)
                PDI->DrawLine(V4, V5, FColor::Green, SDPG_World, 1.0f);
                PDI->DrawLine(V5, V6, FColor::Green, SDPG_World, 1.0f);
                PDI->DrawLine(V6, V7, FColor::Green, SDPG_World, 1.0f);
                PDI->DrawLine(V7, V4, FColor::Green, SDPG_World, 1.0f);

                // Draw vertical edges connecting bottom and top.
                PDI->DrawLine(V0, V4, FColor::Green, SDPG_World, 1.0f);
                PDI->DrawLine(V1, V5, FColor::Green, SDPG_World, 1.0f);
                PDI->DrawLine(V2, V6, FColor::Green, SDPG_World, 1.0f);
                PDI->DrawLine(V3, V7, FColor::Green, SDPG_World, 1.0f);
            }
        }
    }
}

FPrimitiveViewRelevance FCustomBoxSceneProxy::GetViewRelevance(const FSceneView* View) const
{
    FPrimitiveViewRelevance Result;
    Result.bDrawRelevance = IsShown(View);
    Result.bDynamicRelevance = true;
    return Result;
}

SIZE_T FCustomBoxSceneProxy::GetTypeHash() const
{
    static const SIZE_T UniqueTypeHash = FCrc::StrCrc32("FCustomBoxSceneProxy");
    return UniqueTypeHash;
}

uint32 FCustomBoxSceneProxy::GetMemoryFootprint() const
{
    return sizeof(*this) + BoxVertices.GetAllocatedSize();
}