// Fill out your copyright notice in the Description page of Project Settings.

#include "Public/Room.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"

// Sets default values
ARoom::ARoom()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
//	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	//RootComponent = SceneComponent;

	MeshCube = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoomBlock"));
	SetRootComponent(MeshCube);

	//	MeshCube->SetupAttachment(RootComponent);
//	UStaticMesh* m = FObjecfinder

	//MeshCube->SetStaticMesh();
	//UE_LOG(LogTemp, Warning, TEXT("Room Constructor"));
}

// Called when the game starts or when spawned
void ARoom::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Warning, TEXT("Room Begin play"));
}

// tick disabled
// Called every frame
void ARoom::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARoom::SetSimulatePhysicsForAll(bool state)
{
	MeshCube->SetSimulatePhysics(state); 
}

