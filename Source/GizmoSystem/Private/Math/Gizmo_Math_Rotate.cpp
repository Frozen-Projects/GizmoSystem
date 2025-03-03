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

	if (bRotateLocal == true)
	{

	}

	else
	{

	}
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

	if (!IsValid(this->GizmoBase->GizmoTarget))
	{
		if (bEnableDebugMode)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, "Gizmo Rotate : Gizmo target is not valid !");
		}

		return false;
	}

	if (!this->GizmoBase->DetectMovementCallback())
	{
		if (bEnableDebugMode)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, "Gizmo Rotate : There is no movement !");
		}

		return false;
	}

	if (!this->GizmoBase->IsGizmoInViewCallback())
	{
		if (bEnableDebugMode)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, "Gizmo Rotate : Gizmo is not in the view !");
		}

		return false;
	}

	if (this->GizmoBase->ForbiddenKeysCallback())
	{
		if (bEnableDebugMode)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, "Gizmo Rotate : Forbidden key pressed !");
		}

		return false;
	}

	return true;
}