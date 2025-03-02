// Fill out your copyright notice in the Description page of Project Settings.

#include "Custom/vGizmo_UE.h"

ACustomGizmo::ACustomGizmo()
{
    PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    XAxis = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("XAxis"));
    XAxis->SetupAttachment(Root);
    XAxis->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    //XAxis->OnClicked.AddDynamic(this, &ACustomGizmo::OnAxisSelected);

    YAxis = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("YAxis"));
    YAxis->SetupAttachment(Root);
    YAxis->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    //YAxis->OnClicked.AddDynamic(this, &ACustomGizmo::OnAxisSelected);

    ZAxis = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ZAxis"));
    ZAxis->SetupAttachment(Root);
    ZAxis->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    //ZAxis->OnClicked.AddDynamic(this, &ACustomGizmo::OnAxisSelected);
}

void ACustomGizmo::BeginPlay()
{
    Super::BeginPlay();
}

void ACustomGizmo::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void ACustomGizmo::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}