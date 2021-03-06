// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
///////////////////////////////
#include "Tools/DelTraingle/vector2.h"
#include "vector"
#include "Tools/ProceduralState.h"
#include "Tools/DelTraingle/triangle.h"

#include "ProceduralMapsCharacter.generated.h"

class ARoom;

UCLASS(config=Game)
class AProceduralMapsCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	AProceduralMapsCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface
	
	virtual void BeginPlay() override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	virtual void Tick(float deltaTime) override;

	///////////////////////////////////////
	// User defined
	
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Room)
		Pro_States m_State = Pro_States::SpawnRooms;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Room)
		int RoomRange;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Room)
		int m_TotalRoomsToSpawn;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Timer)
		FTimerHandle m_TimerGenerateDT;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Timer)
		float m_TimeForMoveRooms;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Room)
		bool m_StartAlgo = false;

	UPROPERTY(VisibleAnywhere)
	TArray<ARoom*> m_Rooms;
	// main rooms
	TArray<ARoom*> m_RoomsMain;
	// pair with loc and and room for triangle
	TMap<FVector2D, ARoom*> m_RoomLocMap;
	// location pairs generated from MinimumSpanning Tree
	std::vector<std::pair<FVector2D, FVector2D>> m_MinPairs;

	std::vector<dt::Triangle<double>> m_dTriangles;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ARoom> m_SpawningRoom;


	UFUNCTION()
		void OnTimerEnd();

	void RunStates();

	// State func
	UFUNCTION(BlueprintCallable)
		void RunSpawnRoom(); // spawn rooms

	UFUNCTION(BlueprintCallable) // select main rooms
		void RunSperateOverlappingRooms(); // separate rooms

	UFUNCTION(BlueprintCallable) // select main rooms
		void RunHighlightMainRooms();
	
	UFUNCTION(BlueprintCallable) // select main rooms
		void RunDistantiateRooms(float distance); 

	UFUNCTION(BlueprintCallable) // select main rooms
		void RunDrawDelTriangles();

	UFUNCTION(BlueprintCallable) // select main rooms
		void RunDrawMinSpTree();

	UFUNCTION(BlueprintCallable) // select main rooms
		void RunDrawHallways();

};

