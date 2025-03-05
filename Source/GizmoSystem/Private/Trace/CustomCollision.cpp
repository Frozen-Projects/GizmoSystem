#include "Trace/CustomCollision.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/ConvexElem.h"

#include "Misc/Crc.h"

#include "Engine/Engine.h"

UCustomCollision::UCustomCollision(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer), CustomBodySetup(nullptr) // Don't create it here; wait until we have a valid World
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

void UCustomCollision::OnRegister()
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

FPrimitiveSceneProxy* UCustomCollision::CreateSceneProxy()
{
    return new FCustomBoxSceneProxy(this);
}

FBoxSphereBounds UCustomCollision::CalcBounds(const FTransform& LocalToWorld) const
{
    FBox Box(ForceInit);
    for (const FVector& Vertex : Corners)
    {
        Box += Vertex;
    }

    return FBoxSphereBounds(Box.TransformBy(LocalToWorld));
}

UBodySetup* UCustomCollision::GetBodySetup()
{
    return CustomBodySetup;
}

void UCustomCollision::UpdateBodySetup()
{
	
}

void UCustomCollision::UpdateCollision()
{
    if (!CustomBodySetup)
    {
        return;
    }

    // Clear any previous collision geometry.
    CustomBodySetup->AggGeom.ConvexElems.Empty();
    CustomBodySetup->ClearPhysicsMeshes();

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

float UCustomCollision::GetLineThickness() const
{
    return this->LineThickness;
}

#if WITH_EDITOR

void UCustomCollision::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    const FProperty* Property = PropertyChangedEvent.Property;
    
    if (!Property)
    {
        Super::PostEditChangeProperty(PropertyChangedEvent);
        return;
    }

    const FName PropertyName = Property->GetFName().IsNone() ? NAME_None : Property->GetFName();

    if (PropertyName == GET_MEMBER_NAME_CHECKED(UCustomCollision, Corners))
    {
        UpdateCollision();
        MarkRenderStateDirty();
    }

    Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif

bool UCustomCollision::SetExtents(TArray<FVector> New_Corners)
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

// ----------------------------------------------------------------
// FCustomBoxSceneProxy definitions (for debug visualization)
// ----------------------------------------------------------------

void FCustomBoxSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
{
    const FMatrix LocalToWorldMatrix = GetLocalToWorld();
    const int32 NumVerts = BoxVertices.Num();

    const UCustomCollision* MyCollisionComp = static_cast<const UCustomCollision*>(Component);
    const float CurrentLineThickness = MyCollisionComp->GetLineThickness();
    const FColor CurrentShapeColor = MyCollisionComp->ShapeColor; // or use a getter if needed

    // If the thickness is <= 0, do not draw any debug lines.
    if (CurrentLineThickness <= 0.f)
    {
        return;
    }

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
                    PDI->DrawLine(A, B, CurrentShapeColor, SDPG_World, CurrentLineThickness);
                }

                // Draw top polygon.
                for (int32 i = CountPerFace; i < NumVerts; i++)
                {
                    const int32 j = (i + 1 - CountPerFace) % CountPerFace + CountPerFace;
                    const FVector A = LocalToWorldMatrix.TransformPosition(BoxVertices[i]);
                    const FVector B = LocalToWorldMatrix.TransformPosition(BoxVertices[j]);
                    PDI->DrawLine(A, B, CurrentShapeColor, SDPG_World, CurrentLineThickness);
                }

                // Draw vertical edges connecting corresponding vertices.
                for (int32 i = 0; i < CountPerFace; i++)
                {
                    const FVector A = LocalToWorldMatrix.TransformPosition(BoxVertices[i]);
                    const FVector B = LocalToWorldMatrix.TransformPosition(BoxVertices[i + CountPerFace]);
                    PDI->DrawLine(A, B, CurrentShapeColor, SDPG_World, CurrentLineThickness);
                }
            }

            else
            {
                // Fallback: draw a loop connecting all vertices.
                for (int32 i = 0; i < NumVerts; i++)
                {
                    const FVector A = LocalToWorldMatrix.TransformPosition(BoxVertices[i]);
                    const FVector B = LocalToWorldMatrix.TransformPosition(BoxVertices[(i + 1) % NumVerts]);
                    PDI->DrawLine(A, B, CurrentShapeColor, SDPG_World, CurrentLineThickness);
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