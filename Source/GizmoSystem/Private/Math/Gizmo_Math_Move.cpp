#include "Math/Gizmo_Math_Move.h"

// Sets default values.
AGizmoMathMove::AGizmoMathMove()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	this->InitHandles();
}

// Called when the game starts or when spawned.
void AGizmoMathMove::BeginPlay()
{	
	Super::BeginPlay();

	if (!IsValid(this->GetParentActor()))
	{
		return;
	}

	AGizmoMathBase* TempBase = Cast<AGizmoMathBase>(this->GetParentActor());

	if (!IsValid(TempBase))
	{
		return;
	}

	this->GizmoBase = TempBase;
	
	UWorld* CurrentWorld = GEngine->GetCurrentPlayWorld();
	this->PlayerController = UGameplayStatics::GetPlayerController(CurrentWorld, this->GizmoBase->PlayerIndex);
	
	this->BindDelegates();
}

void AGizmoMathMove::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

// Called every frame.
void AGizmoMathMove::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	this->TransformSystem();
}

void AGizmoMathMove::InitHandles()
{
	this->Root = CreateDefaultSubobject<USceneComponent>("Root");

	this->Axis_X = CreateDefaultSubobject<UStaticMeshComponent>("Axis_X");
	this->Axis_X->AttachToComponent(this->Root, FAttachmentTransformRules::KeepRelativeTransform);
	this->Axis_X->ComponentTags.Add(FName("Axis_X"));
	this->Axis_X->SetGenerateOverlapEvents(true);
	this->Axis_X->SetCollisionProfileName(FName("BlockAll"));
	this->Axis_X->SetNotifyRigidBodyCollision(true);
	this->Axis_X->SetCastShadow(false);
	this->Axis_X->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/GizmoSystem/Content/Meshes/SM_Gizmo_Move_Axis.SM_Gizmo_Move_Axis")));

	this->Axis_Y = CreateDefaultSubobject<UStaticMeshComponent>("Axis_Y");
	this->Axis_Y->AttachToComponent(this->Root, FAttachmentTransformRules::KeepRelativeTransform);
	this->Axis_Y->SetRelativeRotation(FRotator3d(0, 90, 0));
	this->Axis_Y->ComponentTags.Add(FName("Axis_Y"));
	this->Axis_Y->SetGenerateOverlapEvents(true);
	this->Axis_Y->SetCollisionProfileName(FName("BlockAll"));
	this->Axis_Y->SetNotifyRigidBodyCollision(true);
	this->Axis_Y->SetCastShadow(false);
	this->Axis_Y->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/GizmoSystem/Content/Meshes/SM_Gizmo_Move_Axis.SM_Gizmo_Move_Axis")));
	
	this->Axis_Z = CreateDefaultSubobject<UStaticMeshComponent>("Axis_Z");
	this->Axis_Z->AttachToComponent(this->Root, FAttachmentTransformRules::KeepRelativeTransform);
	this->Axis_Z->SetRelativeRotation(FRotator3d(90, 0, 0));
	this->Axis_Z->ComponentTags.Add(FName("Axis_Z"));
	this->Axis_Z->SetGenerateOverlapEvents(true);
	this->Axis_Z->SetCollisionProfileName(FName("BlockAll"));
	this->Axis_Z->SetNotifyRigidBodyCollision(true);
	this->Axis_Z->SetCastShadow(false);
	this->Axis_Z->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/GizmoSystem/Content/Meshes/SM_Gizmo_Move_Axis.SM_Gizmo_Move_Axis")));
}

void AGizmoMathMove::TransformSystem()
{
	if (!this->Transform_Check())
	{
		return;
	}

	MoveMultiplier = this->GetInputAxisKeyValue(EKeys::MouseWheelAxis) + MoveMultiplier;

	if (MoveMultiplier <= 0)
	{
		MoveMultiplier = 1;
	}

	if (bMoveLocal)
	{
		this->Transform_Local();
	}

	else
	{
		this->Transform_World();
	}

	this->Transform_Track();
}

bool AGizmoMathMove::Transform_Check()
{
	if (!IsValid(GizmoBase))
	{
		if (bEnableDebugMode)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, "Gizmo Move : Gizmo base is not valid !");
		}

		return false;
	}

	if (!IsValid(this->GizmoBase->GizmoTarget))
	{
		if (bEnableDebugMode)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, "Gizmo Move : Gizmo target is not valid !");
		}

		return false;
	}

	if (!this->GizmoBase->DetectMovementCallback())
	{
		if (bEnableDebugMode)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, "Gizmo Move : There is no movement !");
		}

		return false;
	}

	if (!this->GizmoBase->IsGizmoInViewCallback())
	{
		if (bEnableDebugMode)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, "Gizmo Move : Gizmo is not in the view !");
		}
		
		return false;
	}

	if (this->GizmoBase->ForbiddenKeysCallback())
	{
		if (bEnableDebugMode)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, "Gizmo Move : Forbidden key pressed !");
		}

		return false;
	}

	return true;
}

void AGizmoMathMove::Transform_World()
{
	this->GetRootComponent()->SetWorldRotation(FQuat(0.f), false, nullptr, ETeleportType::None);

	const double ObjectDistance = UKismetMathLibrary::Vector_Distance(this->GizmoBase->PlayerCamera->GetComponentLocation(), this->GizmoBase->GizmoTarget->GetComponentLocation());

	FVector MouseWorldLocation;
	FVector MouseWorldDirection;
	this->PlayerController->DeprojectMousePositionToWorld(MouseWorldLocation, MouseWorldDirection);

	const FVector NewLocation = MouseWorldLocation + (MouseWorldDirection * ObjectDistance);
	const FVector OriginalLocation = this->GizmoBase->GizmoTarget->GetComponentLocation();

	switch (AxisEnum)
	{
		case ESelectedAxis::Null_Axis:
			break;

		case ESelectedAxis::X_Axis:

			this->GizmoBase->GizmoTarget->SetWorldLocation(FVector(NewLocation.X, this->GizmoBase->GizmoTarget->GetComponentLocation().Y, this->GizmoBase->GizmoTarget->GetComponentLocation().Z));
			break;

		case ESelectedAxis::Y_Axis:

			this->GizmoBase->GizmoTarget->SetWorldLocation(FVector(this->GizmoBase->GizmoTarget->GetComponentLocation().X, NewLocation.Y, this->GizmoBase->GizmoTarget->GetComponentLocation().Z));
			break;

		case ESelectedAxis::Z_Axis:

			this->GizmoBase->GizmoTarget->SetWorldLocation(FVector(this->GizmoBase->GizmoTarget->GetComponentLocation().X, this->GizmoBase->GizmoTarget->GetComponentLocation().Y, NewLocation.Z));
			break;

		case ESelectedAxis::XY_Axis:
			break;
		case ESelectedAxis::XZ_Axis:
			break;
		case ESelectedAxis::YZ_Axis:
			break;
		case ESelectedAxis::XYZ_Axis:
			break;
		default:
			break;
	}
}

void AGizmoMathMove::Transform_Local()
{
	if (IsValid(this->GizmoBase->PlayerCamera) == false)
	{
		return;
	}

	if (IsValid(AxisComponent) == false)
	{
		return;
	}

	this->GetRootComponent()->SetWorldRotation(this->GizmoBase->GizmoTarget->GetComponentRotation());

	// MAIN VARIABLES
	double Delta_X = 0;
	double Delta_Y = 0;
	this->PlayerController->GetInputMouseDelta(Delta_X, Delta_Y);

	// MAIN CALCULATIONS
	const FVector AxisForward = AxisComponent->GetForwardVector();
	const FVector NormalTarget = this->GizmoBase->PlayerCamera->GetComponentLocation() - this->GizmoBase->GizmoTarget->GetComponentLocation();
	const FVector NormalizedVector = UKismetMathLibrary::Normal(FVector(NormalTarget.X, NormalTarget.Y, 0.0f), 0.0001);
	const FVector CrossVector = UKismetMathLibrary::Cross_VectorVector(AxisForward, NormalizedVector);
	const double DotProduct = UKismetMathLibrary::Dot_VectorVector(AxisForward, NormalizedVector);

	// X OFFSET
	double HorizontalMultiplier = 0;
	if (CrossVector.Z > 0 == true)
	{
		HorizontalMultiplier = Delta_X * MoveMultiplier;
	}

	else
	{
		HorizontalMultiplier = Delta_X * MoveMultiplier * (-1);
	}

	// Y OFFSET
	double VerticalMultiplier = 0;
	if (UKismetMathLibrary::Abs(AxisForward.Z) >= 0.75 == true)
	{
		if (AxisForward.Z >= 0 == true)
		{
			VerticalMultiplier = Delta_Y * MoveMultiplier;
		}

		else
		{
			VerticalMultiplier = Delta_Y * MoveMultiplier * (-1);
		}

	}

	else
	{
		if (DotProduct > 0 == true)
		{
			VerticalMultiplier = Delta_Y * MoveMultiplier * (-1);
		}

		else
		{
			VerticalMultiplier = Delta_Y * MoveMultiplier;
		}
	}

	FVector DeltaLocation = AxisForward * UKismetMathLibrary::Lerp(VerticalMultiplier, HorizontalMultiplier, UKismetMathLibrary::FClamp(UKismetMathLibrary::Abs(CrossVector.Z), 0, 1));
	this->GizmoBase->GizmoTarget->AddWorldOffset(DeltaLocation, false, nullptr, ETeleportType::None);
}

void AGizmoMathMove::Transform_Track()
{
	USceneComponent* ParentRoot = nullptr;

	if (IsValid(GizmoBase->GetRootComponent()))
	{
		ParentRoot = GizmoBase->GetRootComponent();
		ParentRoot->SetWorldLocation(this->GizmoBase->GizmoTarget->GetComponentLocation(), false, nullptr, ETeleportType::None);
	}
}

void AGizmoMathMove::OnClickedEvent(UPrimitiveComponent* TouchComponent, FKey PressedButton)
{
	if (!IsValid(TouchComponent))
	{
		return;
	}

	this->AxisComponent = TouchComponent;

	if (TouchComponent->ComponentHasTag(FName("Axis_X")))
	{
		this->AxisEnum = ESelectedAxis::X_Axis;
	}

	else if (TouchComponent->ComponentHasTag(FName("Axis_Y")))
	{
		this->AxisEnum = ESelectedAxis::Y_Axis;
	}

	else if (TouchComponent->ComponentHasTag(FName("Axis_Z")))
	{
		this->AxisEnum = ESelectedAxis::Z_Axis;
	}

	if (this->bEnableDebugMode)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, TouchComponent->GetFullName());
	}
}

void AGizmoMathMove::BindDelegates()
{
	if (IsValid(this->Axis_X) && IsValid(this->Axis_Y) && IsValid(this->Axis_X))
	{
		EnableInput(this->PlayerController);
		this->PlayerController->bEnableClickEvents = true;

		this->Axis_X->OnClicked.AddDynamic(this, &AGizmoMathMove::OnClickedEvent);
		this->Axis_Y->OnClicked.AddDynamic(this, &AGizmoMathMove::OnClickedEvent);
		this->Axis_Z->OnClicked.AddDynamic(this, &AGizmoMathMove::OnClickedEvent);		
	}
}

void AGizmoMathMove::SetArrowMesh(UStaticMesh* In_Mesh)
{
	if (!IsValid(In_Mesh))
	{
		return;
	}

	this->Axis_X->SetStaticMesh(In_Mesh);
	this->Axis_Y->SetStaticMesh(In_Mesh);
	this->Axis_Z->SetStaticMesh(In_Mesh);
}
