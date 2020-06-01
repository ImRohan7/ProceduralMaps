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
#include "Engine.h"
////////////////////////////////////
#include "Tools/Generator.h"
#include "Tools/DelTraingle/vector2.h"
#include "Tools/DelTraingle/triangle.h"
#include "Tools/DelTraingle/delaunay.h"
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
	// run state machine
	if(m_StartAlgo)
		RunStates(deltaTime);

}

// simple state machine
void AProceduralMapsCharacter::RunStates(float deltaTime)
{
	switch (m_State)
	{
	case Pro_States::SpawnRooms:	// 1
		RunSpawnRoom();
		break;
	
	case Pro_States::SeparateRooms: // 2
		RunSperateOverlappingRooms();
		break;

	case Pro_States::HighlightMainRooms: // 3
		RunHighlightMainRooms();
		break;

	case Pro_States::DistantiateRooms:
		RunDistantiateRooms(m_MinDist);
		break;

	case Pro_States::DrawDelTriangles:
		RunDrawDelTriangles(); // break
		break;

	case Pro_States::DrawMinSpanTree: // break
		RunDrawMinSpTree();
		break;

	case Pro_States::DrawHallWays: 
		RunDrawHallways();
		break;

	case Pro_States::Waiting:
		m_Timer += deltaTime;
		if (m_Timer >= m_TimeToWait)
		{
			m_State = m_PrevState; // restore state
			m_Timer = 0.f;
		}
		break;

	default:
		break;
	}

}

void AProceduralMapsCharacter::RunSpawnRoom()
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
			rm->m_Scale = scaleX + scaleY;
			m_Rooms.Add(rm);
		}
	}
	else // impoertant Error Logging
	{
		UE_LOG(LogTemp, Error, TEXT("No Actor found while spawning."));
	}

	// set timer for room
	//GetWorldTimerManager().SetTimer(m_TimerGenerateDT, this,
		//&AProceduralMapsCharacter::OnTimerEnd, m_TimeForMoveRooms, false);
	
	// change state
	m_State = Pro_States::SeparateRooms;
}

// sperate overlapping rooms
void AProceduralMapsCharacter::RunSperateOverlappingRooms()
{
	UE_LOG(LogTemp, Warning, TEXT("Separating.............."));

	// do until all are separated
	bool flag = true;
	
	for (auto rm : m_Rooms)
	{
		if (!rm->SeparateOverlappingRooms())
		{
			flag = false;
		}
	}

	if (flag) // chnage if all rooms are done separating
		m_State = Pro_States::HighlightMainRooms;
}


// Highlight main rooms
void AProceduralMapsCharacter::RunHighlightMainRooms()
{
	UE_LOG(LogTemp, Warning, TEXT("Highlighting.............."));

	for (auto rm : m_Rooms)
	{
		if (rm->m_Scale > 14 && 1 == rand() % 4)
		{
			rm->Highlight();	// add
			rm->m_IsMain = true;
			m_RoomsMain.Add(rm);
		}
		else
		{
			if (rand() % 5 > 0)
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
	// change state
	m_State = Pro_States::DistantiateRooms;
}

// Distantiate rooms with aprticular distance
void AProceduralMapsCharacter::RunDistantiateRooms(float distacne)
{
	UE_LOG(LogTemp, Warning, TEXT("Distancing.............."));

	FVector selfLoc, thirdLoc;
	bool flag = true;
	//for each rooms if distance lees than given
	for (auto rm : m_RoomsMain)
	{
		selfLoc = rm->GetActorLocation();
		// check with rm
		for (auto r : m_RoomsMain)
		{
			if (rm != r) // make sure they are not same
			{
				thirdLoc = r->GetActorLocation();

				if (FVector::Distance(selfLoc, thirdLoc) < distacne)
				{
					flag = false;
					// move rooms
					FVector dir = selfLoc - thirdLoc;
					dir.Normalize();
					auto offset = dir * 2;
					rm->AddActorLocalOffset(offset);
					r->AddActorLocalOffset(-offset);
				}
			}
		}
	}
	
	if (flag)
		m_State = Pro_States::DrawDelTriangles;
}

// Draw Deuanay Triangles
void AProceduralMapsCharacter::RunDrawDelTriangles()
{
	UE_LOG(LogTemp, Warning, TEXT("Draw Triangles.............."));
	for (auto r : m_RoomsMain)
	{
		r->updateLocation();
	}
	dt::Delaunay<double> triangulation;
	std::vector<dt::Vector2<double>> points;

	for (auto r : m_RoomsMain)
	{
		dt::Vector2<double> tmp;
		tmp.x = r->m_Loc.X;
		tmp.y = r->m_Loc.Y;
		points.push_back(tmp);
	}

	m_dTriangles = triangulation.triangulate(points);
	m_cTriangles = m_dTriangles;
	UE_LOG(LogTemp, Warning, TEXT("Total Triangles: %d"), m_dTriangles.size());

	// Draw triangles
	dt::Vector2<double> m;
	FVector2D a, b, c;
	float z = 600.f;
	for (auto t : m_dTriangles) // for each triangle
	{
		// get all three rooms
		m = *(t.a);
		a = m.vec();
		m = *(t.b);
		b = m.vec();
		m = *(t.c);
		c = m.vec();

		// draw line for each Edge
		DrawDebugLine(GetWorld(), FVector(a, z), FVector(b, z), FColor::Black,false, m_TimeToWait, 0, 50);
		DrawDebugLine(GetWorld(), FVector(a, z), FVector(c, z), FColor::Black,false, m_TimeToWait, 0, 50);
		DrawDebugLine(GetWorld(), FVector(b, z), FVector(c, z), FColor::Black,false, m_TimeToWait, 0, 50);
		//DrawCircle(GetWorld(), FVector(a, z), FVector(a, z), FVector(a, z), FColor::Red, 40, 2, true);
		
		Mst._costPairs.push_back({ FVector2D::Distance(a, b),
			{a,b} });
		Mst._costPairs.push_back({ FVector2D::Distance(a, c),
			{a,c} });
		Mst._costPairs.push_back({ FVector2D::Distance(b, c),
			{b,c} });
	}

	m_PrevState = Pro_States::DrawMinSpanTree;
	m_State = Pro_States::Waiting;
}

// Generate and Draw Minimum Spanning Tree
void AProceduralMapsCharacter::RunDrawMinSpTree()
{
	UE_LOG(LogTemp, Warning, TEXT("Draw Min Sp Tree.............."));
	float z = 600.f;
	
	UE_LOG(LogTemp, Warning, TEXT("Total pairs in MST: %d"), Mst._costPairs.size());

	m_MinPairs = Mst.getNaturalCostPairs();
	UE_LOG(LogTemp, Warning, TEXT("Extra ballancing MST pairs : %d"), m_MinPairs.size());

	for (auto p : m_MinPairs)
	{
		FVector2D a = p.first;
		FVector2D b = p.second;
		DrawDebugLine(GetWorld(), FVector(a, z + 300), FVector(b, z + 300), FColor::Green, false, m_TimeToWait, 0, 50);
	}

	m_PrevState = Pro_States::DrawHallWays;
	m_State = Pro_States::Waiting;
}

// Generate and Drwa hallways
void AProceduralMapsCharacter::RunDrawHallways()
{
	UE_LOG(LogTemp, Warning, TEXT("Draw Hallways.............."));

	for (auto p : m_MinPairs)
	{
		FVector2D a = p.first;
		FVector2D b = p.second;

		float xDiff = b.X - a.X;
		float yDiff = a.Y - b.Y;
		FVector HorizontalEnd(a.X + xDiff, a.Y, 300);
		FVector VerticalEnd(b.X, b.Y + yDiff, 300);

		DrawDebugLine(GetWorld(), FVector(a, 300), HorizontalEnd, FColor::Blue, true, -1.f, 0, 200);
		DrawDebugLine(GetWorld(), FVector(b, 300), VerticalEnd, FColor::Blue, true, -1.f, 0, 200);
	}
	m_Hallways = m_MinPairs.size();
	m_State = Pro_States::None;
	
}

// Timer End after moving rooms
void AProceduralMapsCharacter::OnTimerEnd()
{
	// make sure rooms are done moving
	RunHighlightMainRooms();
	// Fill map
	for (auto r : m_RoomsMain)
	{
		r->updateLocation();
		m_RoomLocMap.Add(r->m_Loc, r);
	}
	FVector2D g;
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
