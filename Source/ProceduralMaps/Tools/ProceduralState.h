#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"


UENUM(BlueprintType)
enum class Pro_States : uint8
{
	// states for generating procedural map
	SpawnRooms = 0  UMETA(DisplayName = "Spawn all rooms"),
	SeparateRooms = 1  UMETA(DisplayName = "sperate overlapping rooms"),
	HighlightMainRooms = 2  UMETA(DisplayName = "Select Main Rooms"),
	DistantiateRooms = 3  UMETA(DisplayName = "moce rooms to specific distance"),
	DrawDelTriangles = 4  UMETA(DisplayName = "Draw Delaunay Triangles"),
	DrawMinSpanTree = 5  UMETA(DisplayName = "Draw Minimum Spanning Tree"),
	DrawHallWays = 6  UMETA(DisplayName = "Draw Hallways"),
	Waiting = 7,
	None = 8
};