#include "Math/Gizmo_Math_Base.h"
#include "Math/Gizmo_Math_Move.h"

// Sets default values
AGizmoMathBase::AGizmoMathBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"), false);
	RootComponent = Root;

	GizmoType = CreateDefaultSubobject<UChildActorComponent>(TEXT("GizmoType"), false);
	GizmoType->AttachToComponent(this->Root, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
}

// Called when the game starts or when spawned
void AGizmoMathBase::BeginPlay()
{
	Super::BeginPlay();
	
	UWorld* CurrentWorld = GEngine->GetCurrentPlayWorld();
	ACharacter* Character = UGameplayStatics::GetPlayerCharacter(CurrentWorld, PlayerIndex);

	this->PlayerController = UGameplayStatics::GetPlayerController(CurrentWorld, PlayerIndex);
	this->CapsuleComponent = Character->GetCapsuleComponent();

	if (!IsValid(this->PlayerCamera))
	{
		TArray<UCameraComponent*> Components;
		Character->GetComponents(Components);
		for (UCameraComponent* EachComponent : Components)
		{
			UClass* ComponentClass = EachComponent->GetClass();
			if (ComponentClass == UCameraComponent::StaticClass())
			{
				this->PlayerCamera = EachComponent;
				break;
			}
		}
	}

	if (IsValid(this->PlayerCamera))
	{
		EnableInput(this->PlayerController);
		this->PlayerController->bEnableClickEvents = true;
	}

	else
	{
		UE_LOG(LogTemp, Warning, TEXT("You need to define camera and enable input manually."))
	}
}

void AGizmoMathBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

// Called every frame
void AGizmoMathBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Gizmo Size in World.
	if (IsValid(this->CapsuleComponent) && IsValid(GizmoType->GetChildActor()))
	{
		double ScaleAxis = ((FVector::Distance(this->CapsuleComponent->GetComponentLocation(), this->GetRootComponent()->GetComponentLocation())) / this->GizmoSizeMultiplier);
		GizmoType->GetChildActor()->GetRootComponent()->SetWorldScale3D(FVector3d(ScaleAxis, ScaleAxis, ScaleAxis));
	}

	InputComponent->BindAction("AnyKey", IE_Pressed, this, &AGizmoMathBase::AnyKey_Pressed);
	InputComponent->BindAction("AnyKey", IE_Released, this, &AGizmoMathBase::AnyKey_Pressed);
}

void AGizmoMathBase::AnyKey_Pressed(FKey Key)
{
	this->PressedKeys.Add(Key);
}

void AGizmoMathBase::AnyKey_Released(FKey Key)
{
	if (this->PressedKeys.Contains(Key))
	{
		PressedKeys.Remove(Key);
	}
}

bool AGizmoMathBase::ForbiddenKeysCallback()
{
	bool bForbiddenKeyPressed = false;

	for (int32 PressedKeyIndex = 0; PressedKeyIndex <PressedKeys.Num(); PressedKeyIndex++)
	{
		if (ForbiddenKeys.Contains(PressedKeys.Array()[PressedKeyIndex]))
		{
			bForbiddenKeyPressed = true;
			break;
		}
	}

	return bForbiddenKeyPressed;
}

bool AGizmoMathBase::DetectMovementCallback()
{
	double Delta_X;
	double Delta_Y;
	this->PlayerController->GetInputMouseDelta(Delta_X, Delta_Y);

	if (Delta_X || Delta_Y != 0)
	{
		return true;
	}

	else
	{
		return false;
	}
}

bool AGizmoMathBase::IsGizmoInViewCallback()
{
	if (!IsValid(PlayerCamera) || !IsValid(GizmoTarget) || !((UKismetMathLibrary::Dot_VectorVector(PlayerCamera->GetForwardVector(), UKismetMathLibrary::Normal(GizmoTarget->GetComponentLocation() - PlayerCamera->GetComponentLocation(), 0.0001f))) > 0.5))
	{
		return false;
	}

	else
	{
		return true;
	}
}