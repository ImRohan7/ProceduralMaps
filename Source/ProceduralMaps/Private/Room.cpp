// Fill out your copyright notice in the Description page of Project Settings.

#include "Public/Room.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ProceduralMapsCharacter.h"



// Sets default values
ARoom::ARoom()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

//	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	//RootComponent = SceneComponent;

	MeshCube = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoomBlock"));
	SetRootComponent(MeshCube);
	MeshCube->OnComponentBeginOverlap.AddDynamic(this, &ARoom::OnOverlapBeginCube);

	Mat_Orange = CreateDefaultSubobject<UMaterial>(TEXT("Main room material"));
	Mat_Green = CreateDefaultSubobject<UMaterial>(TEXT("Triangle room material"));

	//UE_LOG(LogTemp, Warning, TEXT("Room Constructor"));

}

// Called when the game starts or when spawned
void ARoom::BeginPlay()
{
	Super::BeginPlay();
}

// tick disabled
// Called every frame
void ARoom::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// with overlapping actors
	if (m_ToMove)
	{
		//SeparateOverlappingRooms();
	}
}

/*
	FString fstringVar = "an fstring variable";
	UE_LOG(LogTemp, Warning, TEXT("Text, %d %f %s"), intVar, floatVar, *fstringVar);
*/
void ARoom::SetSimulatePhysicsForAll(bool state)
{
	MeshCube->SetSimulatePhysics(state); 

	
}

void ARoom::OnOverlapBeginCube(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

// Move overlapping Rooms.. Return true if there are no overlapping rooms
bool ARoom::SeparateOverlappingRooms()
{
	bool flag = true;
	TSet<AActor*> outRooms;
	GetOverlappingActors(outRooms, TSubclassOf<ARoom>());
	int32 count = 0;

	AActor* a = Cast<AActor>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	FVector pLoc(735.0, 260.0, 216.0);//a->GetActorLocation();
	auto selfDistance = FVector::Distance(pLoc, GetActorLocation());

	for (AActor* room : outRooms)
	{
		flag = false; // if there are any rooms overlapping
		auto frndLoc = room->GetActorLocation();
		auto frndDistance = FVector::Distance(pLoc, frndLoc);
		auto toCheck = selfDistance < frndDistance ? frndLoc : GetActorLocation();
		auto dir = toCheck - pLoc;
		dir.Normalize();
		FVector offset = dir * 5;
		offset.Z = 0;
		offset.X /= 2;

		offset = selfDistance < frndDistance ? offset : -offset;
		room->AddActorLocalOffset(offset);
		AddActorLocalOffset(-offset);
		count++;
	}

	return flag;
}

void ARoom::Highlight()
{
	MeshCube->SetMaterial(0, Mat_Orange);
}

void ARoom::testMatChange()
{
	MeshCube->SetMaterial(0, Mat_Green);
}

void ARoom::updateLocation()
{
	m_Loc.X = (double)GetActorLocation().X;
	m_Loc.Y = (double)GetActorLocation().Y;
}

