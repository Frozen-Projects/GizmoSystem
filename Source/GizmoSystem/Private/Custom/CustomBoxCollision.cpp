// Fill out your copyright notice in the Description page of Project Settings.

#include "Custom/CustomBoxCollision.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/ConvexElem.h"

#include "Misc/Crc.h"

#include "Engine/Engine.h"

UCustomBoxCollision::UCustomBoxCollision(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer), CustomBodySetup(nullptr) // Don't create it here; wait until we have a valid World
{
    PrimaryComponentTick.bCanEverTick = false;

    // Initialize the eight corners based on Default_Extents.
    Corners.Empty();
    Corners.Add(FVector(-Default_Extents.X, -Default_Extents.Y, -Default_Extents.Z));
    Corners.Add(FVector(Default_Extents.X, -Default_Extents.Y, -Default_Extents.Z));
    Corners.Add(FVector(Default_Extents.X, Default_Extents.Y, -Default_Extents.Z));
    Corners.Add(FVector(-Default_Extents.X, Default_Extents.Y, -Default_Extents.Z));
    Corners.Add(FVector(-Default_Extents.X, -Default_Extents.Y, Default_Extents.Z));
    Corners.Add(FVector(Default_Extents.X, -Default_Extents.Y, Default_Extents.Z));
    Corners.Add(FVector(Default_Extents.X, Default_Extents.Y, Default_Extents.Z));
    Corners.Add(FVector(-Default_Extents.X, Default_Extents.Y, Default_Extents.Z));
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

bool UCustomBoxCollision::SetExtents(TArray<FVector> New_Corners)
{
    if (Corners.Num() < 6 || New_Corners.Num() % 2 != 0)
    {
        return false;
    }

    Corners = New_Corners;

    UpdateCollision();
    MarkRenderStateDirty();

    return true;
}

#if WITH_EDITOR
void UCustomBoxCollision::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    FName PropertyName = (PropertyChangedEvent.Property) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

    // If the Corners array is changed in the editor, update the collision geometry.
    if (PropertyName == GET_MEMBER_NAME_CHECKED(UCustomBoxCollision, Corners))
    {
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
    const FMatrix LocalToWorldMatrix = GetLocalToWorld();
    const int32 NumVerts = BoxVertices.Num();

    for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
    {
        if (VisibilityMap & (1 << ViewIndex))
        {
            FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
            if (NumVerts % 2 == 0 && NumVerts >= 6)
            {
                const int32 CountPerFace = NumVerts / 2;

                // Draw bottom polygon.
                for (int32 i = 0; i < CountPerFace; i++)
                {
                    const FVector A = LocalToWorldMatrix.TransformPosition(BoxVertices[i]);
                    const FVector B = LocalToWorldMatrix.TransformPosition(BoxVertices[(i + 1) % CountPerFace]);
                    PDI->DrawLine(A, B, FColor::Green, SDPG_World, 1.0f);
                }

                // Draw top polygon.
                for (int32 i = CountPerFace; i < NumVerts; i++)
                {
                    const int32 j = (i + 1 - CountPerFace) % CountPerFace + CountPerFace;
                    const FVector A = LocalToWorldMatrix.TransformPosition(BoxVertices[i]);
                    const FVector B = LocalToWorldMatrix.TransformPosition(BoxVertices[j]);
                    PDI->DrawLine(A, B, FColor::Green, SDPG_World, 1.0f);
                }

                // Draw vertical edges connecting corresponding vertices.
                for (int32 i = 0; i < CountPerFace; i++)
                {
                    const FVector A = LocalToWorldMatrix.TransformPosition(BoxVertices[i]);
                    const FVector B = LocalToWorldMatrix.TransformPosition(BoxVertices[i + CountPerFace]);
                    PDI->DrawLine(A, B, FColor::Green, SDPG_World, 1.0f);
                }
            }
            else
            {
                // Fallback: draw a loop connecting all vertices.
                for (int32 i = 0; i < NumVerts; i++)
                {
                    const FVector A = LocalToWorldMatrix.TransformPosition(BoxVertices[i]);
                    const FVector B = LocalToWorldMatrix.TransformPosition(BoxVertices[(i + 1) % NumVerts]);
                    PDI->DrawLine(A, B, FColor::Red, SDPG_World, 1.0f);
                }
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