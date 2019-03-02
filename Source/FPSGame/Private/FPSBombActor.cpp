// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSBombActor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/StaticMeshComponent.h"


// Sets default values
AFPSBombActor::AFPSBombActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ExplodeDelay = 2.0f;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	RootComponent = MeshComp;
}


// Called when the game starts or when spawned
void AFPSBombActor::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle Explode_TimerHandle;
	GetWorldTimerManager().SetTimer(Explode_TimerHandle, this, &AFPSBombActor::Explode, ExplodeDelay);
	
	MatInst = MeshComp->CreateAndSetMaterialInstanceDynamic(0);
	if (MatInst) {
		CurrentColor = MatInst->K2_GetVectorParameterValue("Color");
	}
	TargetColor = FLinearColor::MakeRandomColor();
}

// Called every frame
void AFPSBombActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (MatInst) {
		float Progress = (GetWorld()->TimeSeconds - CreationTime) / ExplodeDelay;
		FLinearColor NewColor = FLinearColor::LerpUsingHSV(CurrentColor, TargetColor, Progress);
		MatInst->SetVectorParameterValue("Color", NewColor);
	}
}

void AFPSBombActor::Explode()
{
	UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionTemplate, GetActorLocation());

	FCollisionObjectQueryParams QueryParams;
	QueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	QueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);

	FCollisionShape CollShape;
	CollShape.SetSphere(500.0f);
	TArray<FOverlapResult> OutOverlap;
	GetWorld()->OverlapMultiByObjectType(OutOverlap, GetActorLocation(), FQuat::Identity, QueryParams, CollShape);

	for (auto& res : OutOverlap) {
		UPrimitiveComponent* Overlap = res.GetComponent();
		if (Overlap && Overlap->IsSimulatingPhysics()) {
			UMaterialInstanceDynamic* MaterialInst = Overlap->CreateAndSetMaterialInstanceDynamic(0);
			if (MaterialInst) {
				MaterialInst->SetVectorParameterValue("Color", TargetColor);
			}
		}
	}

 	Destroy();
}
