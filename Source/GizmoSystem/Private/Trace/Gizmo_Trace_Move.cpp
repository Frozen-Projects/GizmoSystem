#include "Trace/Gizmo_Trace_Move.h"

AGizmoTraceMove::AGizmoTraceMove()
{
    PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    XAxis = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("XAxis"));
    XAxis->SetupAttachment(Root);
    XAxis->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    //XAxis->OnClicked.AddDynamic(this, &AGizmoTraceMove::OnAxisSelected);

    YAxis = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("YAxis"));
    YAxis->SetupAttachment(Root);
    YAxis->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    //YAxis->OnClicked.AddDynamic(this, &AGizmoTraceMove::OnAxisSelected);

    ZAxis = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ZAxis"));
    ZAxis->SetupAttachment(Root);
    ZAxis->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    //ZAxis->OnClicked.AddDynamic(this, &AGizmoTraceMove::OnAxisSelected);
}

void AGizmoTraceMove::BeginPlay()
{
    Super::BeginPlay();
}

void AGizmoTraceMove::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void AGizmoTraceMove::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}