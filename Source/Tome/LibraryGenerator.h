// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"
#include "LibraryGenerator.generated.h"

// Amounts tiles can be rotated on the z-axis
UENUM(BlueprintType)
enum class ETileRotation : uint8
{
	ROT_0   UMETA(DisplayName = "0"),
	ROT_90  UMETA(DisplayName = "90"),
	ROT_180 UMETA(DisplayName = "180"),
	ROT_270 UMETA(DisplayName = "270"),

	ROT_MAX UMETA(Hidden)
};

// Directions tiles are connected
UENUM(BlueprintType)
enum class ETileDirection : uint8
{
	TD_LEFT  UMETA(DisplayName = "Left"),
	TD_RIGHT UMETA(DisplayName = "Right"),
	TD_BACK  UMETA(DisplayName = "Back"),
	TD_FRONT UMETA(DisplayName = "Front"),
	TD_BELOW UMETA(DisplayName = "Below"),
	TD_ABOVE UMETA(DisplayName = "Above"),

	TD_MAX   UMETA(Hidden)
};


// Possible connection types (must match adjacent to generate)
UENUM(BlueprintType)
enum class ETileConnection : uint8
{
	TC_EMPTY                UMETA(DisplayName = "Empty"),
	TC_PATH                 UMETA(DisplayName = "Path"),
	TC_PATH_STAIRS_TOP      UMETA(DisplayName = "PathStairsTop"),
	TC_PATH_STAIRS_TURN_TOP UMETA(DisplayName = "PathStairsTurnTop"),

	TC_MAX                  UMETA(Hidden)
};

// Row from tile connections data table
USTRUCT(BlueprintType)
struct FTileInfo : public FTableRowBase
{
	GENERATED_BODY()

public:
	// Blueprint associated with this tile
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> object;

	// Whether this tile should sometimes generate mirrored
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool mirror;

	// Whether the player can spawn on this tile
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool canSpawnOn;

	// Tiles to not generate adjacent to this tile
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<AActor>> blacklisted;

	// Array of connections for this tile
	UPROPERTY(EditAnywhere)
	ETileConnection connections[uint8(ETileDirection::TD_MAX)];
};

// Active tile instantiated in the world
struct TileInstance
{
	AActor *actor = nullptr; // If null, empty space
	const FTileInfo *info = nullptr; // Pointer to entry in data table
};

// Used internally to keep track of tiles to generate
struct RotatedTile
{
	const FTileInfo *info;
	ETileRotation rot;
	FVector scale;
};

UCLASS()
class TOME_API ALibraryGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALibraryGenerator();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// Convert world coordinates to grid coordinates
	UFUNCTION(BlueprintCallable)
	FIntVector WorldToGrid(FVector world);

	// Convert grid coordinates to world coordinates
	UFUNCTION(BlueprintCallable)
	FVector GridToWorld(FIntVector grid);

	// Get direction in opposite direction of direction
	UFUNCTION(BlueprintCallable)
	ETileDirection ReverseDirection(ETileDirection direction);

	// Get direction rotated on z-axis
	UFUNCTION(BlueprintCallable)
	ETileDirection RotateDirection(ETileDirection direction, ETileRotation rotation);

	// Scales direction (only checks for +/-)
	UFUNCTION(BlueprintCallable)
	ETileDirection ScaleDirection(ETileDirection direction, FVector scale);

	// Switch rotation direction (cw vs ccw)
	UFUNCTION(BlueprintCallable)
	ETileRotation ReverseRotation(ETileRotation rotation);

	// Get a connection of a tile given the direction of the connection and the rotation of the tile
	ETileConnection GetConnection(const FTileInfo *info, ETileDirection direction, ETileRotation rotation = ETileRotation::ROT_0, FVector scale = FVector(1, 1, 1));

	// Convert actor rotation (z-axis) to tile rotation enum
	UFUNCTION(BlueprintCallable)
	ETileRotation ActorTileRotation(AActor *actor);

	// Modulo that preserves repeating pattern in positive numbers
	UFUNCTION(BlueprintCallable)
	int32 PositiveMod(int32 value, int32 mod);

	// Creates a suitable tile in the given position
	UFUNCTION(BlueprintCallable)
	void GenerateTile(FIntVector coord);

	// Adds a tile to the world
	void AddTile(FIntVector coord, ETileRotation direction, FVector scale, const FTileInfo *info);

	// Remove a tile from the world
	UFUNCTION(BlueprintCallable)
	void UnloadTile(FIntVector coord);


public:
	// Data table of tile connection data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable *tileData;

	// Actor to spawn geometry under
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor *geometryParent;

	// Distance within tiles are loaded
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float renderDistance = 10000;

	// Size of each tile
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector gridSize = FVector(2000, 2000, 1000);

	// Draw debug grid?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool debugGridDraw = false;

private:
	// Active tiles in the world
	TMap<FIntVector, TileInstance> tiles_;

	// Corresponds to ETileDirection
	static const FIntVector directions[uint8(ETileDirection::TD_MAX)];

	// Corresponds to ETileRotation
	static const float rotations[uint8(ETileRotation::ROT_MAX)];
};