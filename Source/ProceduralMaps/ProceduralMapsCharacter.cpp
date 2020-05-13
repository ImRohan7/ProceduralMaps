// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ProceduralMapsCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Public/Room.h"
#include "TimerManager.h"

////////////////////////////////////
#include "Tools/Generator.h"
#include "Tools/DelTraingle/vector2.h"
#include "Tools/DelTraingle/triangle.h"
#include "Tools/DelTraingle/delaunay.h"
#include "Tools/MinSpTree/MinSpTree.h"
#include "DrawDebugHelpers.h"

//////////////////////////////////////////////////////////////////////////
// AProceduralMapsCharacter

AProceduralMapsCharacter::AProceduralMapsCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void AProceduralMapsCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AProceduralMapsCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AProceduralMapsCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AProceduralMapsCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AProceduralMapsCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AProceduralMapsCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AProceduralMapsCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AProceduralMapsCharacter::OnResetVR);
}

void AProceduralMapsCharacter::BeginPlay()
{
	Super::BeginPlay();

}

void AProceduralMapsCharacter::Tick(float deltaTime)
{
}

// spawn rooms
void AProceduralMapsCharacter::SpawnRooms()
{
	UE_LOG(LogTemp, Warning, TEXT("Spawning.............."));
	if (m_SpawningRoom)
	{
		for (int i = 0; i < m_TotalRoomsToSpawn; i++)
		{ 
			//spawn
			FActorSpawnParameters tParams;
			tParams.Owner = this;
			FRotator rot;
			//FVector loc = GetActorLocation() + GetActorForwardVector() * 30;
			FVector loc = GetActorLocation() + Helpers::Generator::getRandomPointInCircle(500);
			loc.Z = 226.f;
			rot = FRotator::ZeroRotator;
			ARoom* rm = GetWorld()->SpawnActor<ARoom>(m_SpawningRoom, loc, rot, tParams);

			int scaleX = FMath::RandRange(4, RoomRange);
			int scaleY = FMath::RandRange(4, RoomRange);

			// random scale
			FVector scale(scaleX, scaleY,FMath::RandRange(5, 7));

			rm->SetActorScale3D(scale);
			rm->m_ToMove = true;
			rm->m_Scale = scaleX + scaleY;
			m_Rooms.Add(rm);
		}
	}
	else // impoertant Error Logging
	{
		UE_LOG(LogTemp, Error, TEXT("No Actor found while spawning."));
	}

	// set timer for room
	GetWorldTimerManager().SetTimer(m_TimerGenerateDT, this,
		&AProceduralMapsCharacter::OnTimerEnd, m_TimeForMoveRooms, false);

	// simulate
	for (auto rm : m_Rooms)
	{
		//if(rm)
			//rm->SetSimulatePhysicsForAll(true);
	}

	
}

// color main rooms
void AProceduralMapsCharacter::HighlightMainRooms()
{
	for (auto rm : m_Rooms)
	{
		if (rm->m_Scale > 14 && 1 == rand() % 2)
		{
			rm->Highlight();	// add
			rm->m_IsMain = true;
			m_RoomsMain.Add(rm);
		}
		else
		{
			if (1 != rand() % 3)
			{
				rm->Destroy();
			}
			else
			{
				rm->Highlight();	// add
				rm->m_IsMain = true;
				m_RoomsMain.Add(rm);

			}
		}
	}


	UE_LOG(LogTemp, Warning, TEXT("Remaining Rooms: %d"), m_RoomsMain.Num());
}

// Timer End after moving rooms
void AProceduralMapsCharacter::OnTimerEnd()
{
	// make sure rooms are done moving
	HighlightMainRooms();
	// Fill map
	for (auto r : m_RoomsMain)
	{
		r->updateLocation();
		m_RoomLocMap.Add(r->m_Loc, r);
	}
	FVector2D g;
	
	generateDT();
}

// generate triangle
void AProceduralMapsCharacter::generateDT()
{
	dt::Delaunay<double> triangulation;
	std::vector<dt::Vector2<double>> points;

	for (auto r : m_RoomsMain)
	{
		dt::Vector2<double> tmp;
		tmp.x = r->m_Loc.X;
		tmp.y = r->m_Loc.Y;
		points.push_back(tmp);
	}

	const std::vector<dt::Triangle<double>> triangles = triangulation.triangulate(points);
	UE_LOG(LogTemp, Warning, TEXT("Total Triangles: %d"), triangles.size());

	// Draw triangles
	dt::Vector2<double> m;
	FVector2D a, b, c;
	float z = 600.f;
	for (auto t : triangles) // for each triangle
	{
		// get all three rooms
		m = *(t.a);
		a = m.vec();
		m = *(t.b);
		b = m.vec();
		m = *(t.c);
		c = m.vec();

		// draw line for each Edge
	//	DrawDebugLine(GetWorld(), FVector(a, z), FVector(b, z), FColor::Black,false, 2.f, 0, 50);
	//	DrawDebugLine(GetWorld(), FVector(a, z), FVector(c, z), FColor::Black,false, 2.f, 0, 50);
	//	DrawDebugLine(GetWorld(), FVector(b, z), FVector(c, z), FColor::Black,false, 2.f, 0, 50);
		//DrawCircle(GetWorld(), FVector(a, z), FVector(a, z), FVector(a, z), FColor::Red, 40, 2, true);
	}

	ARoom* s = m_RoomLocMap[a];
	/*s->testMatChange();
	s = m_RoomLocMap[b];
	s->testMatChange();
	s = m_RoomLocMap[c];
	s->testMatChange();*/

	// ********************* MST **************************
	// create minimum spanning tree
	MinSpTree Mst;
	for (auto t : triangles) // for each triangle
	{
		// get all three loc and enter Three as apir
		m = *(t.a);
		a = m.vec();
		m = *(t.b);
		b = m.vec();
		m = *(t.c);
		c = m.vec();

		Mst._costPairs.push_back({ FVector2D::Distance(a, b),
			{a,b} } );
		Mst._costPairs.push_back({ FVector2D::Distance(a, c),
			{a,c} });
		Mst._costPairs.push_back({ FVector2D::Distance(b, b),
			{b,c} });
	}

	UE_LOG(LogTemp, Warning, TEXT("Total pairs in MST: %d"), Mst._costPairs.size());

	auto minPairs = Mst.getMinCostPairs();
	int mp = minPairs.size();
	UE_LOG(LogTemp, Warning, TEXT("After MST pairs : %d"), mp);
	Mst.clear();
	minPairs = Mst.getNaturalCostPairs(); 
	UE_LOG(LogTemp, Warning, TEXT("Extra ballancing MST pairs : %d"), minPairs.size()-mp);

	for (auto p : minPairs)
	{
		FVector2D a = p.first;
		FVector2D b = p.second;
		DrawDebugLine(GetWorld(), FVector(a, z+300), FVector(b, z+300), FColor::Green, true, -1.f, 0, 50);
	}
}

void AProceduralMapsCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AProceduralMapsCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AProceduralMapsCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AProceduralMapsCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AProceduralMapsCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AProceduralMapsCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AProceduralMapsCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
