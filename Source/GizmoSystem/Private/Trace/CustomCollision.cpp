#include "Trace/CustomCollision.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/ConvexElem.h"

#include "Misc/Crc.h"

#include "Engine/Engine.h"

UCustomCollision::UCustomCollision(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = false;

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
    if (!CustomBodySetup)
    {
        CustomBodySetup = NewObject<UBodySetup>(this, NAME_None, RF_Transient);
        CustomBodySetup->CollisionTraceFlag = CTF_UseDefault;
        UpdateCollision();
    }

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
    ConvexElem.UpdateElemBox();
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

TArray<FVector> UCustomCollision::GeneratePyramidVertices(float Height, FVector2D BaseSize)
{
    if (Height <= 0 || BaseSize.X <= 0 || BaseSize.Y <= 0)
    {
        return TArray<FVector>();
    }

    TArray<FVector> Vertices;

    const float HalfBaseX = BaseSize.X / 2.f;
    const float HalfBaseY = BaseSize.Y / 2.f;

    // Base vertices
    Vertices.Add(FVector(-HalfBaseX, -HalfBaseY, 0.f));
    Vertices.Add(FVector(HalfBaseX, -HalfBaseY, 0.f));
    Vertices.Add(FVector(HalfBaseX, HalfBaseY, 0.f));
    Vertices.Add(FVector(-HalfBaseX, HalfBaseY, 0.f));

    // Apex vertex
    Vertices.Add(FVector(0.f, 0.f, Height));

    return Vertices;
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
    if (New_Corners.Num() < 4)
    {
        UE_LOG(LogTemp, Warning, TEXT("You need at least 4 vertices to draw a shape."));
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

FCustomBoxSceneProxy::FCustomBoxSceneProxy(const UCustomCollision* InComponent) : FPrimitiveSceneProxy(InComponent), BoxVertices(InComponent->Corners), Component(InComponent)
{

}

void FCustomBoxSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
{
    const FMatrix LocalToWorldMatrix = GetLocalToWorld();
    const int32 NumVerts = BoxVertices.Num();

    const UCustomCollision* MyCollisionComp = static_cast<const UCustomCollision*>(Component);
    const float CurrentLineThickness = MyCollisionComp->GetLineThickness();
    const FColor CurrentShapeColor = MyCollisionComp->ShapeColor; // or use a getter if needed

    // You need at least 4 vertices to draw a shape. For example triangle bottom and one point at top.
    if (CurrentLineThickness <= 0.f || NumVerts < 4)
    {
        UE_LOG(LogTemp, Warning, TEXT("You need at least 4 vertices and a proper thickness to draw a shape."));
        return;
    }

    for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
    {
        if (VisibilityMap & (1 << ViewIndex))
        {
            FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
            
            // General case: Draw all edges connecting every vertex pair
            for (int32 i = 0; i < NumVerts; ++i)
            {
                for (int32 j = i + 1; j < NumVerts; ++j)
                {
                    const FVector A = LocalToWorldMatrix.TransformPosition(BoxVertices[i]);
                    const FVector B = LocalToWorldMatrix.TransformPosition(BoxVertices[j]);

					// TODO: You can add more conditions to avoid internal lines but this prevents some lines from being drawn in abstract shapes.
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