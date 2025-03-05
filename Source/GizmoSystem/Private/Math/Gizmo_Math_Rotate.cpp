#include "Math/Gizmo_Math_Rotate.h"

AGizmoMathRotate::AGizmoMathRotate()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	this->InitHandles();
}

void AGizmoMathRotate::BeginPlay()
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
	PlayerController->bEnableClickEvents = true;
	EnableInput(this->PlayerController);
}

void AGizmoMathRotate::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AGizmoMathRotate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	this->RotateSystem();
}

void AGizmoMathRotate::InitHandles()
{
	this->Root = CreateDefaultSubobject<USceneComponent>("Root");

	this->Axis_X = CreateDefaultSubobject<UStaticMeshComponent>("Axis_X");
	this->Axis_X->AttachToComponent(this->Root, FAttachmentTransformRules::KeepRelativeTransform);
	this->Axis_X->ComponentTags.Add(FName("Axis_X"));
	this->Axis_X->SetGenerateOverlapEvents(true);
	this->Axis_X->SetCollisionProfileName(FName("BlockAll"));
	this->Axis_X->SetNotifyRigidBodyCollision(true);
	this->Axis_X->SetCastShadow(false);

	this->Axis_Y = CreateDefaultSubobject<UStaticMeshComponent>("Axis_Y");
	this->Axis_Y->AttachToComponent(this->Root, FAttachmentTransformRules::KeepRelativeTransform);
	this->Axis_Y->SetRelativeRotation(FRotator3d(0, 90, 0));
	this->Axis_Y->ComponentTags.Add(FName("Axis_Y"));
	this->Axis_Y->SetGenerateOverlapEvents(true);
	this->Axis_Y->SetCollisionProfileName(FName("BlockAll"));
	this->Axis_Y->SetNotifyRigidBodyCollision(true);
	this->Axis_Y->SetCastShadow(false);

	this->Axis_Z = CreateDefaultSubobject<UStaticMeshComponent>("Axis_Z");
	this->Axis_Z->AttachToComponent(this->Root, FAttachmentTransformRules::KeepRelativeTransform);
	this->Axis_Z->SetRelativeRotation(FRotator3d(90, 0, 0));
	this->Axis_Z->ComponentTags.Add(FName("Axis_Z"));
	this->Axis_Z->SetGenerateOverlapEvents(true);
	this->Axis_Z->SetCollisionProfileName(FName("BlockAll"));
	this->Axis_Z->SetNotifyRigidBodyCollision(true);
	this->Axis_Z->SetCastShadow(false);
}

void AGizmoMathRotate::RotateSystem()
{
	if (!this->Rotate_Check())
	{
		return;
	}

	RotateMultiplier += this->GetInputAxisKeyValue(EKeys::MouseWheelAxis);

	if (RotateMultiplier <= 0)
	{
		RotateMultiplier = 1;
	}

	switch (this->AxisEnum)
	{
		case ESelectedAxis::Null_Axis:
		{
			break;
		}
	
		case ESelectedAxis::X_Axis:
		{
			if (this->bRotateLocal)
			{
				const double Rotation = this->Rotate_XY() * RotateMultiplier;
				this->GizmoBase->GizmoTarget->AddWorldRotation(FRotator(Rotation, 0, 0));
				break;
			}

			else
			{

			}
		}
		
		case ESelectedAxis::Y_Axis:
		{
			if (this->bRotateLocal)
			{
				const double Rotation = this->Rotate_XY() * RotateMultiplier;
				this->GizmoBase->GizmoTarget->AddWorldRotation(FRotator(0, Rotation, 0));
				break;
			}

			else
			{

			}
		}

		case ESelectedAxis::Z_Axis:
		{
			break;
		}

		default:
		{
			break;
		}
	}
}

FVector AGizmoMathRotate::HorizontalNormal(USceneComponent* Target)
{
	const FVector WorldLocation = Target->GetComponentLocation();
	const FVector CameraLocation = this->GizmoBase->PlayerCamera->GetComponentLocation();
	const FVector Difference = CameraLocation - WorldLocation;

	return UKismetMathLibrary::Normal(FVector(Difference.X, Difference.Y, 0));
}

double AGizmoMathRotate::Rotate_XY()
{
	USceneComponent* AxisComp = nullptr;

	if (this->AxisEnum == ESelectedAxis::X_Axis)
	{
		AxisComp = this->Axis_X;
	}

	else if (this->AxisEnum == ESelectedAxis::Y_Axis)
	{
		AxisComp = this->Axis_Y;
	}

	if (!IsValid(AxisComp))
	{
		return 0;
	}

	const FVector AxisForward = AxisComp->GetRightVector();
	const double AxisForward_Z = AxisForward.Z;
	const FVector Normal_X = this->HorizontalNormal(AxisComp);

	const double DotProduct = UKismetMathLibrary::Dot_VectorVector(AxisForward, Normal_X);
	const FVector CrossProduct = UKismetMathLibrary::Cross_VectorVector(AxisForward, Normal_X);
	const double Cross_Z = CrossProduct.Z;
	
	double Mouse_Y_Multiplier_True = 0;

	if (this->AxisEnum == ESelectedAxis::X_Axis)
	{
		Mouse_Y_Multiplier_True = AxisForward_Z >= 0 ? 5 : -5;
	}

	else if (this->AxisEnum == ESelectedAxis::Y_Axis)
	{
		Mouse_Y_Multiplier_True = AxisForward_Z >= 0 ? -5 : 5;
	}
	
	const double Mouse_X_Multiplier = Cross_Z > 0 ? 5 : -5;
	const double Mouse_Y_Multiplier = UKismetMathLibrary::Abs(AxisForward_Z) >= 0.75 ? Mouse_Y_Multiplier_True : (DotProduct > 0 ? -5 : 5);
	const double Mouse_Alpha = UKismetMathLibrary::FClamp(UKismetMathLibrary::Abs(Cross_Z), 0, 1);

	FVector2D MouseDelta = FVector2D((double)0.f);
	this->PlayerController->GetInputMouseDelta(MouseDelta.X, MouseDelta.Y);

	const double Rotation = UKismetMathLibrary::Lerp(Mouse_Y_Multiplier * MouseDelta.Y, Mouse_X_Multiplier * MouseDelta.X, Mouse_Alpha);
	return Rotation;
}

bool AGizmoMathRotate::Rotate_Check()
{
	if (!IsValid(GizmoBase))
	{
		if (bEnableDebugMode)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, "Gizmo Rotate : Gizmo base is not valid !");
		}

		return false;
	}

	else if (!IsValid(this->GizmoBase->GizmoTarget))
	{
		if (bEnableDebugMode)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, "Gizmo Rotate : Gizmo target is not valid !");
		}

		return false;
	}

	else if (!this->GizmoBase->DetectMovementCallback())
	{
		if (bEnableDebugMode)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, "Gizmo Rotate : There is no movement !");
		}

		return false;
	}

	else if (!this->GizmoBase->IsGizmoInViewCallback())
	{
		if (bEnableDebugMode)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, "Gizmo Rotate : Gizmo is not in the view !");
		}

		return false;
	}

	else if (this->GizmoBase->ForbiddenKeysCallback())
	{
		if (bEnableDebugMode)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, "Gizmo Rotate : Forbidden key pressed !");
		}

		return false;
	}

	else if (this->Check_Visibility())
	{
		if (bEnableDebugMode)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, "Gizmo Rotate : Gizmo is not visible !");
		}

		return false;
	}

	return true;
}

bool AGizmoMathRotate::Check_Visibility()
{
	const FVector CameraFowardVector = this->GizmoBase->PlayerCamera->GetForwardVector();
	const FVector CameraLocation = this->GizmoBase->PlayerCamera->GetComponentLocation();
	const FVector TargetLocation = this->GizmoBase->GizmoTarget->GetComponentLocation();

	const FVector Difference = TargetLocation - CameraLocation;
	const FVector NormalizedVector = UKismetMathLibrary::Normal(Difference, 0.0001);
	const double DotProduct = UKismetMathLibrary::Dot_VectorVector(CameraFowardVector, NormalizedVector);

	return DotProduct > 0.5 ? true : false;
}
