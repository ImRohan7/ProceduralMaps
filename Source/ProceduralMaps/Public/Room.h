// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
///////////////////////////////
#include "Room.generated.h"

class UPrimitiveComponent;
class USceneComponent;


UCLASS()
class PROCEDURALMAPS_API ARoom : public AActor
{
	GENERATED_BODY()
	
public:	
	ARoom();
protected:
	virtual void BeginPlay() override;
public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (DisplayName = "Location Scene Coponent"))
		USceneComponent* SceneComponent;

	UPROPERTY(EditAnywhere)
		UMaterial* Mat_Orange;

	UPROPERTY(EditAnywhere)
		UMaterial* Mat_Green;

	// visible in variables section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (DisplayName = "RoomBlockName"))
		UStaticMeshComponent* MeshCube;
	
//	UPROPERTY(VisibleAnywhere) // visible in component hierarchy
	// whether to move rooms
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System", meta = (DisplayName = "ToMoveWithCollision"))
		bool m_ToMove = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System", meta = (DisplayName = "ToMoveWithCollision"))
		bool m_ToSeparate = false;

	// the room scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System", meta = (DisplayName = "ScaleOfRoom"))
		int m_Scale = 0;

	// Is Main Room
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System", meta = (DisplayName = "IsMain"))
		int m_IsMain = false;

	FVector2D m_Loc;

	//**********************************************************
	// Functions
	UFUNCTION(BlueprintCallable)
		void SetSimulatePhysicsForAll(bool state);

	UFUNCTION() // with this macro u must write the definition of the function
		void OnOverlapBeginCube(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


	bool SeparateOverlappingRooms(); // return true when there are no overlapping rooms

	void Highlight();
	void testMatChange();
	void updateLocation();


};
