// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Room.generated.h"

UCLASS()
class PROCEDURALMAPS_API ARoom : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARoom();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


//	UPROPERTY(VisibleAnywhere) // visible in component hierarchy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (DisplayName = "Location Scene Coponent"))
		USceneComponent* SceneComponent;

	// visible in variables section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (DisplayName = "RoomBlockName"))
		UStaticMeshComponent* MeshCube;

	//**********************************************************
	// Functions
	UFUNCTION(BlueprintCallable)
		void SetSimulatePhysicsForAll(bool state);


};
